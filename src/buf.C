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
	pageNumber = -1;
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
	pages = new Page[numbuf];
	descriptors = new Descriptor[numbuf];
}

// **********************************************************
// BufMgr class destructor
BufMgr::~BufMgr() {
	delete[] pages;
	delete[] descriptors;

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

	MINIBASE_DB->allocate_page(firstPageId, howmany);
	return OK;
}

// **********************************************************
Status BufMgr::freePage(PageId globalPageId) {
	int pos=getPagePos(globalPageId);
	if(pos>=0){
		if(descriptors[pos].pinCount>0){
			return MINIBASE_FIRST_ERROR(BUFMGR,PAGE_PINNED);
		}
	}
	Status st = MINIBASE_DB->allocate_page(globalPageId, 1);
	if (st != OK) {
		return MINIBASE_CHAIN_ERROR(BUFMGR, st);
	} else {
		return st;
	}
}

// **********************************************************
Status BufMgr::flushPage(PageId pageId) {
	Page* page = getPage(pageId);
	if (page != NULL) {
		Status st = MINIBASE_DB->write_page(pageId, page);
		if (st != OK) {
			return MINIBASE_CHAIN_ERROR(BUFMGR, st);
		} else {
			return st;
		}
	}
	return MINIBASE_FIRST_ERROR(BUFMGR,PAGE_NOT_FOUND);
}

// **********************************************************
Status BufMgr::flushAllPages() {
	for (unsigned int i = 0; i < poolsize; i++) {
		if (descriptors[i].pageNumber >= 0) {
			Status result = flushPage(descriptors[i].pageNumber);
			if (result != OK) {
				return MINIBASE_CHAIN_ERROR(BUFMGR, result);
			}
		}
	}
	return OK;
}

Page* BufMgr::getPage(PageId pageid) {
	for (unsigned int i = 0; i < poolsize; i++) {
		if (descriptors[i].pageNumber == poolsize) {
			return &(pages[i]);
		}
	}
	return NULL;
}
int BufMgr::getPagePos(PageId pageid) {
	for (unsigned int i = 0; i < poolsize; i++) {
		if (descriptors[i].pageNumber == poolsize) {
			return i;
		}
	}
	return -1;
}

// **********************************************************
// DO NOT REMOVE THIS METHOD
Status BufMgr::pinPage(PageId PageId_in_a_DB, Page*& page, int emptyPage) {
	// please do not remove this line ========================
	emptyPage = 0;
	// =======================================================
	return pinPage(PageId_in_a_DB, page, READ_MODE);
}
