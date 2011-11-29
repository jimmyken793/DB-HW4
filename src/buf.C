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

int hash(int i) {
	return i % HTSIZE;
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

	poolsize = numbuf;
	frames = new Page[numbuf];
	descs = new FrameDesc[numbuf];
}

// **********************************************************
// BufMgr class destructor
BufMgr::~BufMgr() {
	delete[] frames;
	delete[] descs;
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
	int frame = getFreeFrame();
	if (frame < 0) {
		return MINIBASE_FIRST_ERROR(BUFMGR, BUFFER_FULL);
	}
	Status ast = MINIBASE_DB->allocate_page(firstPageId, howmany);
	if (ast != OK)
		return MINIBASE_CHAIN_ERROR(BUFMGR, ast);

	descs[frame].init(firstPageId, READ_WRITE_MODE, 1, false);
	firstPage = &frames[frame];
	return OK;
	return OK;
}

// **********************************************************
Status BufMgr::freePage(PageId globalPageId) {
	int pos = getFrame(globalPageId);
	if (pos >= 0) {
		if (descs[pos].pinCount > 0) {
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
		if (descs[i].pageNumber >= 0) {
			Status result = flushPage(descs[i].pageNumber);
			if (result != OK) {
				return MINIBASE_CHAIN_ERROR(BUFMGR, result);
			}
		}
	}
	return OK;
}

Page* BufMgr::getPage(PageId pageid) {
	for (unsigned int i = 0; i < poolsize; i++) {
		if (descs[i].pageNumber == poolsize) {
			return &(frames[i]);
		}
	}
	return NULL;
}
int BufMgr::getFrame(PageId pageid) {
	for (unsigned int i = 0; i < poolsize; i++) {
		if (descs[i].pageNumber == poolsize) {
			return i;
		}
	}
	return -1;
}

int BufMgr::getFreeFrame() {
	for (unsigned int i = 0; i < poolsize; i++) {
		if (descs[i].pageNumber < 0) {
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
