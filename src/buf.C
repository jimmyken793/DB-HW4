/*****************************************************************************/
/*************** Implementation of the Buffer Manager Layer ******************/
/*****************************************************************************/

#include "buf.h"

// Define buffer manager error messages

static const char* bufErrMsgs[] = {
// todo: error message strings go here
        };

// Create a static "error_string_table" object and register the error messages
// with minibase system 

static error_string_table bufTable(BUFMGR, bufErrMsgs);

// todo: Design your own hash table and replacement policy here.

Descriptor::Descriptor() {
	pageNumber = 0;
	mode = 0;
	pinCount = 0;
	dirtybit = 0;
}

// **********************************************************
// Class Buffer Manager
// **********************************************************

// **********************************************************
// BufMgr class constructor
BufMgr::BufMgr(int numbuf, Replacer* replacer) {
	// DO NOT REMOVE THESE TWO LINE ==============================
	minibase_globals->DummyBufMgr = this;
	replacer = 0; // DISREGARD THE PARAMETER
	// ===========================================================
	bufPool = new Page[numbuf];
	descPool = new Descriptor[numbuf];
}

// **********************************************************
// BufMgr class destructor
BufMgr::~BufMgr() {
	delete[] bufPool;
}

Status BufMgr::pinPage(PageId PageId_in_a_DB, Page*& page, MODE mode) {

	return OK;
}

// **********************************************************
Status BufMgr::unpinPage(PageId globalPageId_in_a_DB, int dirty = FALSE, int hate = FALSE) {

	return OK;
}

// **********************************************************
Status BufMgr::newPage(PageId& firstPageId, Page*& firstPage, int howmany) {
	// DO NOT REMOVE THIS LINE =========================
	howmany = 1;
	// ================================================
	return MINIBASE_DB->allocate_page(PageId& firstPageId, howmany);
}

// **********************************************************
Status BufMgr::freePage(PageId globalPageId) {

	return OK;
}

// **********************************************************
Status BufMgr::flushPage(PageId pageId) {
	// todo: fill the body
	return OK;
}

// **********************************************************
Status BufMgr::flushAllPages() {
	// todo: fill the body
	return OK;
}

// **********************************************************
// DO NOT REMOVE THIS METHOD
Status BufMgr::pinPage(PageId PageId_in_a_DB, Page*& page, int emptyPage) {
	// please do not remove this line ========================
	emptyPage = 0;
	// =======================================================
	return pinPage(PageId_in_a_DB, page, READ_MODE);
}
