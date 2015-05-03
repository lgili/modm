// coding: utf-8
/* Copyright (c) 2013, Roboterclub Aachen e.V.
 * All Rights Reserved.
 *
 * The file is part of the xpcc library and is released under the 3-clause BSD
 * license. See the file `LICENSE` for the full license governing this code.
 */
// ----------------------------------------------------------------------------

#ifndef XPCC_LIS3_TRANSPORT_HPP
#	error  "Don't include this file directly, use 'lis3_transport.hpp' instead!"
#endif

// ============================================================================
// MARK: I2C TRANSPORT
template < class I2cMaster >
xpcc::Lis3TransportI2c<I2cMaster>::Lis3TransportI2c(uint8_t address)
:	I2cDevice<I2cMaster, 2>(address)
{
}

// MARK: - register access
// MARK: write register
template < class I2cMaster >
xpcc::co::ResumableResult<bool>
xpcc::Lis3TransportI2c<I2cMaster>::write(uint8_t reg, uint8_t value)
{
	CO_BEGIN();

	buffer[0] = reg;
	buffer[1] = value;

	this->transaction.configureWrite(buffer, 2);

	CO_END_RETURN_CALL( this->runTransaction() );
}

// MARK: read register
template < class I2cMaster >
xpcc::co::ResumableResult<bool>
xpcc::Lis3TransportI2c<I2cMaster>::read(uint8_t reg, uint8_t *buffer, uint8_t length)
{
	CO_BEGIN();

	this->buffer[0] = reg;
	this->transaction.configureWriteRead(this->buffer, 1, buffer, length);

	CO_END_RETURN_CALL( this->runTransaction() );
}

// ============================================================================
// MARK: SPI TRANSPORT
template < class SpiMaster, class Cs >
xpcc::Lis3TransportSpi<SpiMaster, Cs>::Lis3TransportSpi(uint8_t /*address*/)
:	whoAmI(0)
{
	Cs::setOutput(xpcc::Gpio::High);
}

// MARK: ping
template < class SpiMaster, class Cs >
xpcc::co::ResumableResult<bool>
xpcc::Lis3TransportSpi<SpiMaster, Cs>::ping()
{
	CO_BEGIN();

	whoAmI = 0;

	CO_CALL(read(0x0F, whoAmI));

	CO_END_RETURN(whoAmI != 0);
}

// MARK: - register access
// MARK: write register
template < class SpiMaster, class Cs >
xpcc::co::ResumableResult<bool>
xpcc::Lis3TransportSpi<SpiMaster, Cs>::write(uint8_t reg, uint8_t value)
{
	CO_BEGIN();

	CO_WAIT_UNTIL(this->aquireMaster());
	Cs::reset();

	CO_CALL(SpiMaster::transfer(reg | Write));
	CO_CALL(SpiMaster::transfer(value));

	if (this->releaseMaster())
		Cs::set();

	CO_END_RETURN(true);
}

// MARK: read register
template < class SpiMaster, class Cs >
xpcc::co::ResumableResult<bool>
xpcc::Lis3TransportSpi<SpiMaster, Cs>::read(uint8_t reg, uint8_t *buffer, uint8_t length)
{
	CO_BEGIN();

	CO_WAIT_UNTIL(this->aquireMaster());
	Cs::reset();

	CO_CALL(SpiMaster::transfer(reg | Read));

	CO_CALL(SpiMaster::transfer(nullptr, buffer, length));

	if (this->releaseMaster())
		Cs::set();

	CO_END_RETURN(true);
}

