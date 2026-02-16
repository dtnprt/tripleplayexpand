//
// simpleencoder.cpp
//
// mt32-pi - A baremetal MIDI synthesizer for Raspberry Pi
// Copyright (C) 2020-2023 Dale Whinham <daleyo@gmail.com>
//
// This file is part of mt32-pi.
//
// mt32-pi is free software: you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the Free Software
// Foundation, either version 3 of the License, or (at your option) any later
// version.
//
// mt32-pi is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
// details.
//
// You should have received a copy of the GNU General Public License along with
// mt32-pi. If not, see <http://www.gnu.org/licenses/>.
//

#include "control/button.h"
#include "control/control.h"

constexpr u8 GPIOPinButton1 = 17;
constexpr u8 GPIOPinButton2 = 27;

constexpr u8 GPIOPinEncoder1Button = 4;
constexpr u8 GPIOPinEncoder1CLK    = 22;
constexpr u8 GPIOPinEncoder1DAT    = 23;

constexpr u8 GPIOPinEncoder2Button = 26;
constexpr u8 GPIOPinEncoder2CLK    = 16;
constexpr u8 GPIOPinEncoder2DAT    = 12;

constexpr u8 ButtonMask = 1 << static_cast<u8>(TButton::Button1) |
			  1 << static_cast<u8>(TButton::Button2) |
			  1 << static_cast<u8>(TButton::EncoderButton1) |
			  1 << static_cast<u8>(TButton::EncoderButton2);

CControlSimpleEncoder::CControlSimpleEncoder(TEventQueue& pEventQueue, CRotaryEncoder::TEncoderType EncoderType, bool bEncoderReversed)
	: CControl(pEventQueue),

	  m_GPIOEncoderButton_1(GPIOPinEncoder1Button, TGPIOMode::GPIOModeInputPullUp),
	  m_GPIOEncoderButton_2(GPIOPinEncoder2Button, TGPIOMode::GPIOModeInputPullUp),

	  m_GPIOButton1(GPIOPinButton1, TGPIOMode::GPIOModeInputPullUp),
	  m_GPIOButton2(GPIOPinButton2, TGPIOMode::GPIOModeInputPullUp),

	  m_Encoder_1(EncoderType, bEncoderReversed, GPIOPinEncoder1CLK, GPIOPinEncoder1DAT),
	  m_Encoder_2(EncoderType, bEncoderReversed, GPIOPinEncoder2CLK, GPIOPinEncoder2DAT)
	  
{
}

void CControlSimpleEncoder::Update()
{
	CControl::Update();

	const s8 nEncoderDelta_1 = m_Encoder_1.Read();
	if (nEncoderDelta_1 != 0)
	{
		TEvent Event;
		Event.Type = TEventType::Encoder;
		Event.Encoder.Encoder = TEncoder::Encoder1;
		Event.Encoder.nDelta = nEncoderDelta_1;
		m_pEventQueue->Enqueue(Event);
	}
	const s8 nEncoderDelta_2 = m_Encoder_2.Read();
	if (nEncoderDelta_2 != 0)
	{
		TEvent Event;
		Event.Type = TEventType::Encoder;
		Event.Encoder.Encoder = TEncoder::Encoder2;
		Event.Encoder.nDelta = nEncoderDelta_2;
		m_pEventQueue->Enqueue(Event);
	}
}

void CControlSimpleEncoder::ReadGPIOPins()
{
	// Read current button state from GPIO pins
	const u32 nGPIOState  = CGPIOPin::ReadAll();
	const u8 nButtonState = (((nGPIOState >> GPIOPinButton1) & 1) << static_cast<u8>(TButton::Button1)) |
				(((nGPIOState >> GPIOPinButton2) & 1) << static_cast<u8>(TButton::Button2)) |
				(((nGPIOState >> GPIOPinEncoder1Button) & 1) << static_cast<u8>(TButton::EncoderButton1)) |
				(((nGPIOState >> GPIOPinEncoder2Button) & 1) << static_cast<u8>(TButton::EncoderButton2))
				;

	DebounceButtonState(nButtonState, ButtonMask);

	// Update rotary encoder state
	m_Encoder_1.ReadGPIOPins((nGPIOState >> GPIOPinEncoder1CLK) & 1, (nGPIOState >> GPIOPinEncoder1DAT) & 1);
	m_Encoder_2.ReadGPIOPins((nGPIOState >> GPIOPinEncoder2CLK) & 1, (nGPIOState >> GPIOPinEncoder2DAT) & 1);
}
