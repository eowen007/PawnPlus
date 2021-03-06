#include "../natives.h"
#include "../tasks.h"
#include "../context.h"

typedef void(*logprintf_t)(char* format, ...);
extern logprintf_t logprintf;

namespace Natives
{
	// native task:wait_ticks(ticks);
	static cell AMX_NATIVE_CALL wait_ticks(AMX *amx, cell *params)
	{
		if(params[1] == 0) return 0;
		amx_RaiseError(amx, AMX_ERR_SLEEP);
		if(params[1] < 0)
		{
			return SleepReturnWaitInf;
		}else{
			return SleepReturnWaitTicks | (SleepReturnValueMask & params[1]);
		}
	}

	// native task:wait_ms(interval);
	static cell AMX_NATIVE_CALL wait_ms(AMX *amx, cell *params)
	{
		if(params[1] == 0) return 0;
		amx_RaiseError(amx, AMX_ERR_SLEEP);
		if(params[1] < 0)
		{
			return SleepReturnWaitInf;
		}else{
			return SleepReturnWaitMs | (SleepReturnValueMask & params[1]);
		}
	}

	// native task:task_new();
	static cell AMX_NATIVE_CALL task_new(AMX *amx, cell *params)
	{
		return TaskPool::CreateNew().Id();
	}

	// native task_set_result(task:task, AnyTag:result);
	static cell AMX_NATIVE_CALL task_set_result(AMX *amx, cell *params)
	{
		auto task = TaskPool::Get(params[1]);
		if(task != nullptr)
		{
			task->SetCompleted(params[2]);
			return 1;
		}
		return 0;
	}

	// native task_get_result(task:task);
	static cell AMX_NATIVE_CALL task_get_result(AMX *amx, cell *params)
	{
		auto task = TaskPool::Get(params[1]);
		if(task != nullptr)
		{
			return task->Result();
		}
		return 0;
	}

	// native task:task_ticks(ticks);
	static cell AMX_NATIVE_CALL task_ticks(AMX *amx, cell *params)
	{
		if(params[1] <= 0)
		{
			auto &task = TaskPool::CreateNew();
			if(params[1] == 0)
			{
				task.SetCompleted(0);
			}
			return task.Id();
		}else{
			auto &task = TaskPool::CreateTickTask(params[1]);
			return task.Id();
		}
	}

	// native task:task_ms(interval);
	static cell AMX_NATIVE_CALL task_ms(AMX *amx, cell *params)
	{
		if(params[1] <= 0)
		{
			auto &task = TaskPool::CreateNew();
			if(params[1] == 0)
			{
				task.SetCompleted(0);
			}
			return task.Id();
		}else{
			auto &task = TaskPool::CreateTimerTask(params[1]);
			return task.Id();
		}
	}

	// native task_await(task:task);
	static cell AMX_NATIVE_CALL task_await(AMX *amx, cell *params)
	{
		task_id id = static_cast<task_id>(params[1]);
		auto task = TaskPool::Get(id);
		if(task != nullptr)
		{
			if(!task->Completed())
			{
				amx_RaiseError(amx, AMX_ERR_SLEEP);
				return SleepReturnAwait | (id & SleepReturnValueMask);
			}else{
				return task->Result();
			}
		}
		return 0;
	}

	// native task_yield(AnyTag:value);
	static cell AMX_NATIVE_CALL task_yield(AMX *amx, cell *params)
	{
		auto &ctx = Context::Get(amx);
		ctx.result = params[1];
		if(ctx.task_object != -1)
		{
			TaskPool::Get(ctx.task_object)->SetCompleted(ctx.result);
			return 1;
		}
		return 0;
	}
}

static AMX_NATIVE_INFO native_list[] =
{
	AMX_DECLARE_NATIVE(wait_ticks),
	AMX_DECLARE_NATIVE(wait_ms),
	AMX_DECLARE_NATIVE(task_new),
	AMX_DECLARE_NATIVE(task_set_result),
	AMX_DECLARE_NATIVE(task_get_result),
	AMX_DECLARE_NATIVE(task_ticks),
	AMX_DECLARE_NATIVE(task_ms),
	AMX_DECLARE_NATIVE(task_await),
	AMX_DECLARE_NATIVE(task_yield),
};

int RegisterTasksNatives(AMX *amx)
{
	return amx_Register(amx, native_list, sizeof(native_list) / sizeof(*native_list));
}
