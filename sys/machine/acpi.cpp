//
//  acpi.cpp
//  Firedrake
//
//  Created by Sidney Just
//  Copyright (c) 2014 by Sidney Just
//  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated 
//  documentation files (the "Software"), to deal in the Software without restriction, including without limitation 
//  the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, 
//  and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, 
//  INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR 
//  PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE 
//  FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, 
//  ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//

#include <kern/kprintf.h>
#include <libc/string.h>
#include <machine/interrupts/apic.h>
#include "acpi.h"

namespace ACPI
{
	constexpr uint64_t RSDPSignature = UINT64_C(0x2052545020445352);
	static RSDP *_rsdp = nullptr;

	RSDT *RSDP::GetRSDT() const
	{
		static bool resolvedOtherSDT = false;

		RSDT *rsdt = reinterpret_cast<RSDT *>(rsdtAddress);

		if(!resolvedOtherSDT)
		{
			rsdt->otherSDT = reinterpret_cast<uint32_t *>(&rsdt->otherSDT);
			resolvedOtherSDT = true;
		}

		return rsdt;
	}

	RSDTHeader *RSDT::FindHeader(const char *signature) const
	{
		size_t entries = (length - sizeof(RSDTHeader)) / 4;

		for(size_t i = 0; i < entries; i ++)
		{
			RSDTHeader *header = reinterpret_cast<RSDTHeader *>(otherSDT[i]);

			if(strncmp(signature, reinterpret_cast<char *>(header->signature), 4) == 0)
				return header;
		}

		return nullptr;
	}

	kern_return_t RSDT::ParseMADT() const
	{
		MADT *madt = FindEntry<MADT>("APIC");
		bool hasBootstrapCPU = false;

		if(madt)
		{
			MADTEntry *entry = madt->GetFirstEntry();
			size_t entries = madt->GetEntryCount();

			for(size_t i = 0; i < entries; i ++)
			{
				switch(entry->type)
				{
					case 0:
					{
						MADTApic *apic = static_cast<MADTApic *>(entry);
						uint8_t enabled = (apic->flags & 0x1);

						if(enabled)
						{
							Sys::CPU::Flags flags = 0;

							if(!hasBootstrapCPU)
							{
								flags = Sys::CPU::Flags::Bootstrap | Sys::CPU::Flags::Running;
								hasBootstrapCPU = true;
							}

							Sys::CPU cpu(apic->apicID, flags);
							cpu.Register();
						}

						break;
					}

					case 1:
					{
						MADTIOApic *ioapic = static_cast<MADTIOApic *>(entry);
						ir::ioapic_t ioapic_entry;

						ioapic_entry.id = ioapic->ioapicID;
						ioapic_entry.address = ioapic->ioapicAddress;
						ioapic_entry.interrupt_base = ioapic->interruptBase;

						kern_return_t result = apic_add_ioapic(&ioapic_entry);
						if(result != KERN_SUCCESS)
							return result;

						break;
					}

					case 2:
					{
						MADTInterruptSource *isos = static_cast<MADTInterruptSource *>(entry);

						ir::ioapic_interrupt_override_t override;
						override.source = isos->source;
						override.system_interrupt = isos->globalSystemInterrupt;
						override.flags = isos->flags;

						apic_add_interrupt_override(&override);
						break;
					}
				}

				entry = entry->GetNext();
			}

			return KERN_SUCCESS;
		}

		return KERN_FAILURE;
	}


	size_t MADT::GetEntryCount() const
	{
		size_t entries = (length - sizeof(MADT)) / 4;
		return entries;
	}

	MADTEntry *MADT::GetFirstEntry() const
	{
		return reinterpret_cast<MADTEntry *>(const_cast<MADT *>(this + 1));
	}

	MADTEntry *MADTEntry::GetNext() const
	{
		uint8_t *temp = reinterpret_cast<uint8_t *>(const_cast<MADTEntry *>(this));
		return reinterpret_cast<MADTEntry *>(temp + length);
	}





	RSDP *FindRSDPInRange(uintptr_t start, size_t size)
	{
		uint16_t *pointer  = reinterpret_cast<uint16_t *>(start);
		uint16_t *rangeEnd = reinterpret_cast<uint16_t *>(start + size);

		while(pointer != rangeEnd)
		{
			RSDP *rsdp = reinterpret_cast<RSDP *>(pointer);
			if(rsdp->signature == RSDPSignature)
			{
				uint8_t *temp = reinterpret_cast<uint8_t *>(rsdp);
				uint8_t *end  = reinterpret_cast<uint8_t *>(rsdp + 1);

				// Calculate the checksum
				uint8_t checksum = 0;
				while(temp != end)
					checksum += *temp ++;

				if(checksum == 0)
					return rsdp;
			}

			pointer ++;
		}

		return nullptr;
	}

	RSDP *FindRSDP()
	{
		if((_rsdp = FindRSDPInRange(0xe0000, 0x20000)))
			return _rsdp;

		return nullptr;
	}

	kern_return_t Init()
	{
		if(!FindRSDP())
		{
			kprintf("Couldn't find ACPI table");
			return KERN_FAILURE;
		}

		kern_return_t result;
		RSDT *rsdt = _rsdp->GetRSDT();

		if((result = rsdt->ParseMADT()) != KERN_SUCCESS)
		{
			kprintf("Failed to parse MADT");
			return result;
		}

		return KERN_SUCCESS;
	}
}
