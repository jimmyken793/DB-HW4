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
	int frame = table.get(PageId_in_a_DB);
	if (frame >= 0) {
		if (descs[frame].pinCount == 0) {
			hatelist.remove(frame);
			lovelist.remove(frame);
		} else {
			if (descs[frame].mode == READ_WRITE_MODE || descs[frame].mode == READ_WRITE_MODE)
				return MINIBASE_FIRST_ERROR(BUFMGR, PIN_LOCKED_PAGE);
		}
		descs[frame].dirty = false;
		descs[frame].pinCount++;
		page = &(frames[frame]);
		return OK;
	} else {
		frame = getFreeFrame();
		if (frame > 0) {
			Status st = MINIBASE_DB->read_page(PageId_in_a_DB, &frames[frame]);
			if (st != OK)
				return MINIBASE_CHAIN_ERROR(BUFMGR, st);
			table.put(PageId_in_a_DB, frame);
			descs[frame].pageNumber = PageId_in_a_DB;
		} else {
			return MINIBASE_FIRST_ERROR(BUFMGR, POOL_FULL);
		}
	}
	return OK;
}

// **********************************************************
Status BufMgr::unpinPage(PageId globalPageId_in_a_DB, int dirty = FALSE, int hate = FALSE) {
	int frame = table.get(globalPageId_in_a_DB);
	if (frame >= 0) {
		if (descs[frame].pinCount == 0) {
			return MINIBASE_FIRST_ERROR(BUFMGR, PIN_COUNTE_RROR);
		}
		if (dirty) {
			Status st = MINIBASE_DB->write_page(descs[frame].pageNumber, &frames[frame]);
			if (st != OK)
				return MINIBASE_CHAIN_ERROR(BUFMGR, st);
		}
		descs[frame].dirty = dirty;
		descs[frame].pinCount--;
		if (descs[frame].pinCount > 0) {
			return OK;
		}
		if (hate == true) {
			bool found = false;
			for (list<int>::iterator iter = lovelist.begin(); iter != lovelist.end(); iter++) {
				if (*iter == frame) {
					found = true;
					break;
				}
			}
			if (!found) {
				hatelist.remove(frame);
				hatelist.push_front(frame);
			}
		} else {
			for (list<int>::iterator iter = hatelist.begin(); iter != hatelist.end(); iter++) {
				if (*iter == frame) {
					hatelist.remove(frame);
					break;
				}
			}
			lovelist.remove(frame);
			lovelist.push_front(frame);
		}
	} else {
		return MINIBASE_FIRST_ERROR(BUFMGR, PAGE_NOT_FOUND);
	}
	return OK;
}

// **********************************************************
Status BufMgr::newPage(PageId& firstPageId, Page*& firstPage, int howmany) {
	// DO NOT REMOVE THIS LINE =========================
	howmany = 1;
	int frame = findFreeFrame();
	if (frame < 0) {
		return MINIBASE_FIRST_ERROR(BUFMGR, BUFFER_FULL);
	}
	Status st = MINIBASE_DB->allocate_page(firstPageId, howmany);
	if (st != OK)
		return MINIBASE_CHAIN_ERROR(BUFMGR, st);
	descs[frame].init(firstPageId, READ_WRITE_MODE, 1, false);
	firstPage = &frames[frame];
	table.put(firstPageId, frame);
	return OK;
}

// **********************************************************
Status BufMgr::freePage(PageId globalPageId) {
	int pos = getFrame(globalPageId);
	if (pos >= 0) {
		if (descs[pos].pinCount > 0) {
			return MINIBASE_FIRST_ERROR(BUFMGR,PAGE_PINNED);
		}
		table.remove(globalPageId);
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
	int frame = getFrame(pageid);
	if (frame >= 0) {
		return &(frames[frame]);
	}
	return NULL;
}
int BufMgr::getFrame(PageId pageid) {
	return table.get(pageid);
}

int BufMgr::findFreeFrame() {
	for (unsigned int i = 0; i < poolsize; i++) {
		if (descs[i].pageNumber < 0) {
			return i;
		}
	}
	return -1;
}
int BufMgr::getFreeFrame() {
	int frame = findFreeFrame();
	if (frame >= 0)
		return frame;
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
