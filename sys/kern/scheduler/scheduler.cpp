//
//  scheduler.cpp
//  Firedrake
//
//  Created by Sidney Just
//  Copyright (c) 2013 by Sidney Just
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

#include <libc/assert.h>
#include <kern/kprintf.h>
#include <libc/string.h>
#include <libcpp/algorithm.h>
#include <machine/interrupts/interrupts.h>
#include <machine/clock/clock.h>
#include <kern/panic.h>
#include "scheduler.h"

namespace sd
{
	void kernel_task()
	{
		kprintf("Hello World\n");

		while(1)
		{}
	}

	void idle_task()
	{
		kprintf("idle task on CPU %i\n", cpu_get_current_cpu()->id);

		while(1)
			cpu_halt();
	}


	scheduler_t::scheduler_t() :
		_proxy_cpu_count(cpu_get_cpu_count()),
		_thread_lock(SPINLOCK_INIT)
	{
		std::fill(_proxy_cpus, _proxy_cpus + CPU_MAX_CPUS, nullptr);

		for(size_t i = 0; i < _proxy_cpu_count; i ++)
		{
			cpu_proxy_t *proxy = new cpu_proxy_t(cpu_get_cpu_with_id(i));
			_proxy_cpus[i] = proxy;
		}
	}

	void scheduler_t::initialize()
	{
		create_kernel_task();
		create_idle_task();
	}

	void scheduler_t::create_kernel_task()
	{
		_kernel_task = new task_t(vm::get_kernel_directory());
		_kernel_task->attach_thread(nullptr, (thread_t::entry_t)&kernel_task, 0);
	}

	void scheduler_t::create_idle_task()
	{
		_idle_task = new task_t(vm::get_kernel_directory());
		
		for(size_t i = 0; i < _proxy_cpu_count; i ++)
		{
			cpu_proxy_t *proxy = _proxy_cpus[i];
			thread_t *thread;

			if(_idle_task->attach_thread(&thread, (thread_t::entry_t)&idle_task, 0) != KERN_SUCCESS)
				panic("Failed to create idle thread");

			thread->_pinned_cpu  = proxy->cpu;
			thread->_running_cpu = proxy->cpu;

			proxy->idle_thread   = thread;
			proxy->active_thread = thread;
		}
	}


	task_t *scheduler_t::get_active_task()
	{
		thread_t *thread = get_active_thread();
		return thread->get_task();
	}

	task_t *scheduler_t::get_active_task(cpu_t *cpu)
	{
		thread_t *thread = get_active_thread(cpu);
		return thread->get_task();
	}

	thread_t *scheduler_t::get_active_thread()
	{
		cpu_t *cpu = cpu_get_current_cpu();
		cpu_proxy_t *proxy = _proxy_cpus[cpu->id];

		return proxy->active_thread;
	}

	thread_t *scheduler_t::get_active_thread(cpu_t *cpu)
	{
		if(cpu == cpu_get_current_cpu())
			return get_active_thread();

		cpu_proxy_t *proxy = _proxy_cpus[cpu->id];

		spinlock_lock(&proxy->lock);
		thread_t *thread = proxy->active_thread;
		spinlock_unlock(&proxy->lock);

		return thread;
	}


	void scheduler_t::add_thread(thread_t *thread)
	{
		spinlock_lock(&_thread_lock);
		_active_threads.insert_back(thread->_scheduler_entry);
		spinlock_unlock(&_thread_lock);
	}

	uint32_t scheduler_t::reschedule_on_cpu(uint32_t esp, cpu_t *cpu)
	{
		cpu_proxy_t *proxy = _proxy_cpus[cpu->id];
		thread_t *thread = proxy->active_thread;

		thread->_quantum = 0;

		return schedule_on_cpu(esp, cpu);
	}

	uint32_t scheduler_t::schedule_on_cpu(uint32_t esp, cpu_t *cpu)
	{
		cpu_proxy_t *proxy = _proxy_cpus[cpu->id];

		if(__expect_false(!proxy->active) || __expect_false(!spinlock_try_lock(&proxy->lock)))
			return esp;

		thread_t *thread = proxy->active_thread;
		thread->lock();

		if(__expect_true(!proxy->first_run))
			thread->_esp = esp;

		if((thread->_quantum --) <= 0 || !thread->is_schedulable(cpu))
		{
			// Find a new thread/task pair
			thread_t *original = thread;
			cpp::queue<thread_t>::entry *entry = &thread->_scheduler_entry;
			bool found = false;

			thread->unlock();

			do {
				spinlock_lock(&_thread_lock);

				entry = entry->get_next();
				if(!entry)
					entry = _active_threads.get_head();

				spinlock_unlock(&_thread_lock);

				thread = entry->get();
				thread->lock();

				if(thread->is_schedulable(cpu))
				{
					found = true;
					break;
				}

				thread->unlock();
			} while(thread != original);

			if(!found) // Default to the idle thread if we are starved of threads
			{
				thread = proxy->idle_thread;
				thread->lock();
			}

			// Move the new thread onto this CPU
			proxy->active_thread = thread;
			thread->_running_cpu  = cpu;
			
			// TODO: Calculate a better quantum for the thread
			thread->_quantum = 5;
			thread->unlock();

			// Mark the old thread as no longer running on this CPU
			original->lock();
			original->_running_cpu = nullptr;
			original->unlock();
		}
		else
		{
			thread->unlock();
		}

		proxy->first_run = false;
		spinlock_unlock(&proxy->lock);

		task_t *task = thread->get_task();
		ir::trampoline_cpu_data_t *data = cpu->data;

		data->page_directory = task->get_directory()->get_physical_directory();
		data->tss.esp0       = thread->_esp + sizeof(cpu_state_t);

		return thread->_esp;
	}

	void scheduler_t::activate_cpu(cpu_t *cpu)
	{
		cpu_proxy_t *proxy = _proxy_cpus[cpu->id];
		assert(proxy && proxy->active == false);

		proxy->active    = true;
		proxy->first_run = true;

		proxy->active_thread = proxy->idle_thread;
		proxy->active_thread->_quantum = 1;
	}



	uint32_t timer_interrupt_stub(uint32_t esp, cpu_t *cpu)
	{
		return scheduler_t::get_shared_instance()->schedule_on_cpu(esp, cpu);
	}

	uint32_t activate_cpu(uint32_t esp, cpu_t *cpu)
	{
		ir::apic_set_timer(clock::default_time_divisor, ir::apic_timer_mode_t::periodic, clock::default_time_counter);
		ir::apic_arm_timer(clock::default_time_counter);

		scheduler_t::get_shared_instance()->activate_cpu(cpu);
		return scheduler_t::get_shared_instance()->schedule_on_cpu(esp, cpu);
	}

	kern_return_t init()
	{
		scheduler_t *scheduler = scheduler_t::get_shared_instance();
		scheduler->initialize();

		ir::set_interrupt_handler(0x35, &timer_interrupt_stub);
		ir::set_interrupt_handler(0x3a, &activate_cpu);

		return KERN_SUCCESS;
	}
}
