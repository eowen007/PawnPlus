#include "reset.h"
#include "fixes/linux.h"
#include <cstring>

typedef void(*logprintf_t)(char* format, ...);
extern logprintf_t logprintf;

AMX_RESET::AMX_RESET(AMX* amx) : context(Context::TryGet(amx)), amx(amx), cip(amx->cip), frm(amx->frm), pri(amx->pri), alt(amx->alt), hea(amx->hea), reset_hea(amx->reset_hea), stk(amx->stk), reset_stk(amx->reset_stk)
{
	unsigned char *dat, *h, *s;

	auto amxhdr = (AMX_HEADER*)amx->base;
	dat = amx->base + amxhdr->dat;
	h = amx->base + amxhdr->hea;
	s = dat + stk;

	size_t heap_size = hea - (amxhdr->hea - amxhdr->dat);
	heap = std::make_unique<unsigned char[]>(heap_size);
	std::memcpy(heap.get(), h, heap_size);

	size_t stack_size = amxhdr->stp - amxhdr->dat - stk;
	stack = std::make_unique<unsigned char[]>(stack_size);
	std::memcpy(stack.get(), s, stack_size);
}

void AMX_RESET::restore() const
{
	restore_no_context();
	Context::Restore(amx, context);
}

void AMX_RESET::restore_no_context() const
{
	unsigned char *dat, *h, *s;

	auto amxhdr = (AMX_HEADER*)amx->base;
	dat = amx->base + amxhdr->dat;
	h = amx->base + amxhdr->hea;
	s = dat + stk;

	amx->cip = cip;
	amx->frm = frm;
	amx->pri = pri;
	amx->alt = alt;
	amx->hea = hea;
	amx->reset_hea = hea;
	amx->stk = stk;
	amx->reset_stk = stk;

	size_t heap_size = hea - (amxhdr->hea - amxhdr->dat);
	std::memcpy(h, heap.get(), heap_size);

	size_t stack_size = amxhdr->stp - amxhdr->dat - stk;
	std::memcpy(s, stack.get(), stack_size);
}

AMX_RESET::AMX_RESET(AMX_RESET &&obj) : context(obj.context), amx(obj.amx), cip(obj.cip), frm(obj.frm), pri(obj.pri), alt(obj.alt), hea(obj.hea), reset_hea(obj.reset_hea), heap(std::move(obj.heap)), stk(obj.stk), reset_stk(obj.reset_stk), stack(std::move(obj.stack))
{

}

AMX_RESET::AMX_RESET(const AMX_RESET &obj) : context(obj.context), amx(obj.amx), cip(obj.cip), frm(obj.frm), pri(obj.pri), alt(obj.alt), hea(obj.hea), reset_hea(obj.reset_hea), stk(obj.stk), reset_stk(obj.reset_stk)
{
	auto amxhdr = (AMX_HEADER*)amx->base;
	size_t heap_size = hea - (amxhdr->hea - amxhdr->dat);
	heap = std::make_unique<unsigned char[]>(heap_size);
	std::memcpy(heap.get(), obj.heap.get(), heap_size);

	size_t stack_size = amxhdr->stp - amxhdr->dat - stk;
	stack = std::make_unique<unsigned char[]>(stack_size);
	std::memcpy(stack.get(), obj.stack.get(), stack_size);
}

AMX_RESET &AMX_RESET::operator=(AMX_RESET &&obj)
{
	if(this == &obj) return *this;

	context = obj.context;
	amx = obj.amx;
	cip = obj.cip, frm = obj.frm, pri = obj.pri, alt = obj.alt;
	stk = obj.stk, reset_stk = obj.reset_stk;
	hea = obj.hea, reset_hea = obj.reset_hea;
	heap = std::move(obj.heap);
	stack = std::move(obj.stack);
	return *this;
}

AMX_RESET &AMX_RESET::operator=(const AMX_RESET &obj)
{
	if(this == &obj) return *this;

	context = obj.context;
	amx = obj.amx;
	cip = obj.cip, frm = obj.frm, pri = obj.pri, alt = obj.alt;
	stk = obj.stk, reset_stk = obj.reset_stk;
	hea = obj.hea, reset_hea = obj.reset_hea;

	auto amxhdr = (AMX_HEADER*)amx->base;
	size_t heap_size = hea - (amxhdr->hea - amxhdr->dat);
	heap = std::make_unique<unsigned char[]>(heap_size);
	std::memcpy(heap.get(), obj.heap.get(), heap_size);

	size_t stack_size = amxhdr->stp - amxhdr->dat - stk;
	stack = std::make_unique<unsigned char[]>(stack_size);
	std::memcpy(stack.get(), obj.stack.get(), stack_size);
	return *this;
}
