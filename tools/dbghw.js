//Fake debug hardware for game jam

console.log("Fake debug hardware");

const DBG_MAGIC_ADDR = 0xBFF00000;
const DBG_TEXTLEN_ADDR = 0xBFF00004;
const DBG_TEXTBUF_ADDR = 0xBFF00008;

const DBG_MAGIC_VAL = 0x44424748; //DBGH

const DBGHW_ADDR = new AddressRange(DBG_MAGIC_ADDR, DBG_TEXTBUF_ADDR+3);

//Basic variables for simulating reads
var return_data = 0;
var return_reg = 0;
var callbackId = 0;


events.onread(DBGHW_ADDR, function(addr) {
	return_reg = getStoreOp();
	return_data = 0;
	if(addr == DBG_MAGIC_ADDR) {
		return_data = DBG_MAGIC_VAL;
	}
	callbackId = events.onexec((gpr.pc + 4), ReadCartReg);
});

var buf_addr = 0;
var text_buf = [];

events.onwrite(DBGHW_ADDR, function(addr) {
	if(addr == DBG_TEXTLEN_ADDR) {
		var len = getStoreOpValue();
		text_buf = [];
		for(var i=0; i<len; i++) {
			var value = mem.u8[buf_addr+i];
			if(value == 10) {
				text_buf.push(13);
			}
			text_buf.push(value);
		}
		text_buf.push(0);
		console.print(String.fromCharCode.apply(null, text_buf));
	} else if(addr == DBG_TEXTBUF_ADDR) {
		buf_addr = getStoreOpValue();
	}
});

function getStoreOp()
{
	// hacky way to get value that SW will write
	var pcOpcode = mem.u32[gpr.pc];
	var tReg = (pcOpcode >> 16) & 0x1F;
	return tReg;
}

function getStoreOpValue()
{
	// hacky way to get value that SW will write
	var pcOpcode = mem.u32[gpr.pc];
	var tReg = (pcOpcode >> 16) & 0x1F;
	return gpr[tReg];
}

function ReadCartReg()
{
    gpr[return_reg] = return_data;
    events.remove(callbackId);
}