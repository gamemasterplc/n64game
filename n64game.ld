OUTPUT_ARCH (mips)

//Starts a segment
#define BEGIN_SEG(name, addr) \
	_##name##SegmentStart = ADDR(.name); \
	_##name##SegmentRomStart = __romPos; \
	.name addr : AT(__romPos)

//Ends a segment
#define END_SEG(name) \
	_##name##SegmentRomEnd = __romPos + SIZEOF(.name); \
	_##name##SegmentEnd = ADDR(.name) + SIZEOF(.name); \
	__romPos += SIZEOF(.name);

//Starts a noload segment which is intended to contain variables which do not have data yet.
#define BEGIN_NOLOAD(name) \
	_##name##SegmentBssStart = ADDR(.name.noload); \
	.name.noload (NOLOAD) :

//Ends a noload segment
#define END_NOLOAD(name) \
	_##name##SegmentBssEnd = ADDR(.name.noload) + SIZEOF(.name.noload); \
	_##name##SegmentEnd = ADDR(.name.noload) + SIZEOF(.name.noload); \
	_##name##SegmentBssSize = SIZEOF(.name.noload);
	
#define INCOBJ_LOAD(path) \
	path(.text*); \
	path(.gnu.linkonce.t*); \
	path(.rodata*) \
	path(.gnu.linkonce.r*); \
	path(.data*); \
	path(.gnu.linkonce.d*) 
	
#define INCOBJ_NOLOAD(path) \
	path(COMMON); \
	path(.scommon); \
	path(.bss*); \
	path(.gnu.linkonce.b*)
	
#define INCOBJ(path) \
	INCOBJ_LOAD(path); \
	INCOBJ_NOLOAD(path)
	
SECTIONS
{
	__romPos = 0;

	BEGIN_SEG(boot, 0x04000000)
	{
		KEEP(BUILD_DIR/asm/rom_header.o(.text)); /* ROM Header */
		KEEP(BUILD_DIR/boot.6102.o(.data)); /* CIC 6102 bootcode */
	}
	END_SEG(boot)
	ASSERT(__romPos == 0x1000, "Incorrect header size")
	
	BEGIN_SEG(main, 0x80000400)
	{
		BUILD_DIR/asm/entry.o(.start);
		INCOBJ_LOAD(BUILD_DIR/src*.o);
		INCOBJ_LOAD(/usr/lib/n64/nusys/nusys_rom.o);
		INCOBJ_LOAD(*/libnusys.a:*.o);
		INCOBJ_LOAD(*/libultra_rom.a:*.o);
		INCOBJ_LOAD(*/libgcc.a:*.o);
		INCOBJ_LOAD(/usr/lib/n64/PR/rspboot.o);
		INCOBJ_LOAD(/usr/lib/n64/PR/gspF3DEX2.fifo.o);
		INCOBJ_LOAD(/usr/lib/n64/PR/gspL3DEX2.fifo.o);
		INCOBJ_LOAD(/usr/lib/n64/PR/gspF3DEX2.Rej.fifo.o);
		INCOBJ_LOAD(/usr/lib/n64/PR/gspF3DEX2.NoN.fifo.o);
		INCOBJ_LOAD(/usr/lib/n64/PR/gspF3DLX2.Rej.fifo.o);
		INCOBJ_LOAD(/usr/lib/n64/PR/gspS2DEX2.fifo.o);
	}
	END_SEG(main)
	ASSERT(__romPos <= 0x101000, "Main segment too big")
	
	. = ALIGN(8);
	
	BEGIN_NOLOAD(main)
	{
		INCOBJ_NOLOAD(BUILD_DIR/src*.o);
		INCOBJ_NOLOAD(/usr/lib/n64/nusys/nusys_rom.o);
		INCOBJ_NOLOAD(*/libnusys.a:*.o);
		INCOBJ_NOLOAD(*/libultra_rom.a:*.o);
		INCOBJ_NOLOAD(*/libgcc.a:*.o);
		. = ALIGN(8);
	}
	END_NOLOAD(main)
	
	//Align heap to 8 bytes
	. = ALIGN(8);
	_heap_start = .;
	
	BEGIN_SEG(bin_data, __romPos)
	{
		BUILD_DIR/asm/bin_data.o(.data); 
	}
	END_SEG(bin_data)
	
	//Discard everything not specifically mentioned above.
	/DISCARD/ :
	{
		*(*);
	}
}
