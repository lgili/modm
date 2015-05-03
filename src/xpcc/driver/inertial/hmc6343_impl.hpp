// coding: utf-8
/* Copyright (c) 2013, Roboterclub Aachen e.V.
 * All Rights Reserved.
 *
 * The file is part of the xpcc library and is released under the 3-clause BSD
 * license. See the file `LICENSE` for the full license governing this code.
 */
// ----------------------------------------------------------------------------

#ifndef XPCC_HMC6343_HPP
#	error  "Don't include this file directly, use 'hmc6343.hpp' instead!"
#endif
#include <xpcc/math/utils/operator.hpp>

// ----------------------------------------------------------------------------
template < class I2cMaster >
xpcc::Hmc6343<I2cMaster>::Hmc6343(Data &data, uint8_t address)
:	I2cDevice<I2cMaster,2>(address), data(data), timeout(500)
{
}

// ----------------------------------------------------------------------------
// MARK: - register access
// MARK: write command
template < class I2cMaster >
xpcc::co::ResumableResult<bool>
xpcc::Hmc6343<I2cMaster>::writeCommand(Command command, uint16_t timeout)
{
	CO_BEGIN();

	buffer[0] = i(command);
	CO_WAIT_UNTIL( this->timeout.isExpired() and this->startWrite(buffer, 1) );

	this->timeout.restart(timeout);
	CO_WAIT_WHILE( this->isTransactionRunning() );

	CO_END_RETURN( this->wasTransactionSuccessful() );
}

// MARK: write register
template < class I2cMaster >
xpcc::co::ResumableResult<bool>
xpcc::Hmc6343<I2cMaster>::writeRegister(Register reg, uint8_t value)
{
	CO_BEGIN();

	buffer[0] = i(Command::WriteEeprom);
	buffer[1] = i(reg);
	buffer[2] = value;

	CO_WAIT_UNTIL( timeout.isExpired() and this->startWrite(buffer, 3) );

	timeout.restart(10);
	CO_WAIT_WHILE( this->isTransactionRunning() );

	CO_END_RETURN( this->wasTransactionSuccessful() );
}

// MARK: write 16bit register
template < class I2cMaster >
xpcc::co::ResumableResult<bool>
xpcc::Hmc6343<I2cMaster>::writeRegister(Register16 reg, uint16_t value)
{
	CO_BEGIN();

	// LSB
	if ( CO_CALL( writeRegister(static_cast<Register>(reg), value) ) )
	{
		// MSB
		CO_RETURN_CALL( writeRegister(static_cast<Register>(i(reg)+1), (value >> 8)) );
	}

	CO_END_RETURN(false);
}

// MARK: read register
template < class I2cMaster >
xpcc::co::ResumableResult<bool>
xpcc::Hmc6343<I2cMaster>::readRegister(Register reg, uint8_t &value)
{
	CO_BEGIN();

	buffer[0] = i(Command::ReadEeprom);
	buffer[1] = i(reg);
	CO_WAIT_UNTIL( timeout.isExpired() and this->startWrite(buffer, 2) );

	timeout.restart(10);
	CO_WAIT_WHILE( this->isTransactionRunning() );

	if( this->wasTransactionSuccessful() )
	{
		CO_WAIT_UNTIL( timeout.isExpired() and this->startRead(&value, 1) );

		CO_WAIT_WHILE( this->isTransactionRunning() );

		CO_RETURN( this->wasTransactionSuccessful() );
	}

	CO_END_RETURN(false);
}

// MARK: read 16bit register
template < class I2cMaster >
xpcc::co::ResumableResult<bool>
xpcc::Hmc6343<I2cMaster>::readRegister(Register16 reg, uint16_t &value)
{
	CO_BEGIN();

	// LSB
	if ( CO_CALL( readRegister(static_cast<Register>(reg), buffer[2]) ) )
	{
		// MSB
		value = buffer[2];
		if ( CO_CALL( readRegister(static_cast<Register>(i(reg)+1), buffer[2]) ) )
		{
			// an optimization would be to take the uint8_t address of the MSB value
			// but then we would have to deal with endianess, and that headache is annoying.
			value |= (buffer[2] << 8);
			CO_RETURN(true);
		}
	}

	CO_END_RETURN(false);
}

// MARK: read 6 or 1 bytes of data
template < class I2cMaster >
xpcc::co::ResumableResult<bool>
xpcc::Hmc6343<I2cMaster>::readPostData(Command command, uint8_t offset, uint8_t readSize)
{
	CO_BEGIN();

	if (CO_CALL(writeCommand(command, 1)))
	{
		CO_WAIT_UNTIL( timeout.isExpired() and this->startRead(data.data + offset, readSize) );

		CO_WAIT_WHILE( this->isTransactionRunning() );

		CO_RETURN( this->wasTransactionSuccessful() );
	}

	CO_END_RETURN(false);
}
