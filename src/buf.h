///////////////////////////////////////////////////////////////////////////////
/////////////  The Header File for the Buffer Manager /////////////////////////
///////////////////////////////////////////////////////////////////////////////

#ifndef BUF_H
#define BUF_H

#include "db.h"
#include "page.h"
#include <list>

#define NUMBUF 20   
// Default number of frames, artifically small number for ease of debugging.

#define HTSIZE 7
// Hash Table size

int hash(int i);

/*******************ALL BELOW are purely local to buffer Manager********/

// You should create enums for internal errors in the buffer manager.
enum bufErrCodes {
	PAGE_NOT_FOUND, PAGE_PINNED, BUFFER_FULL
};

class Replacer;

enum MODE {
	READ_MODE, READ_WRITE_MODE
};
class Map {
	public:
		int get(PageId id) {
			int pos = id % HTSIZE;
			for (list<pair<PageId, int> >::const_iterator iter = table[pos].begin(); iter != table[pos].end(); iter++) {
				if (iter->first == id) {
					return iter->second;
				}
			}
			return -1;
		}
		void put(PageId pid, int id) {
			int pos = pid % HTSIZE;
			table[pos].push_back(make_pair(pid, id));
		}
		int remove(PageId id) {
			if (id < 0)
				return 0;
			int pos = id % HTSIZE;
			for (list<pair<PageId, int> >::iterator iter = table[pos].begin(); iter != table[pos].end(); iter++) {
				if (iter->first == id) {
					table[pos].erase(iter);
					return 1;
				}
			}
			return 0;
		}
	private:
		list<pair<PageId, int> > table[HTSIZE];
};

class FrameDesc {
	public:
		FrameDesc() {
			init();
		}

		void init() {
			init(-1, READ_MODE, 0, false);
		}
		void init(unsigned int pageNumber, MODE mode, unsigned int pinCount, bool dirty) {
			this->pageNumber = pageNumber;
			this->mode = mode;
			this->pinCount = pinCount;
			this->dirty = dirty;
		}
		unsigned int pageNumber, pinCount;
		MODE mode;
		bool dirty;
};

class BufMgr {
	private:
		unsigned int poolsize;
		Page* frames;
		FrameDesc* descs;
		list<int> hatelist;
		list<int> lovelist;

	public:
		// The physical buffer pool of pages.

		BufMgr(int numbuf, Replacer *replacer = 0);
		// Initializes a buffer manager managing "numbuf" buffers.
		// Disregard the "replacer" parameter for now. In the full 
		// implementation of minibase, it is a pointer to an object
		// representing one of several buffer pool replacement schemes.

		~BufMgr();
		// Should flush all dirty pages in the pool 
		// to disk before shutting down 
		// and deallocate the buffer pool in main memory. 

		Status pinPage(PageId PageId_in_a_DB, Page*& page, MODE mode);
		// Check if this page is in buffer pool. If it is, increment the
		// pin_count and let page be a pointer to this page.
		// If the pin_count was 0 before the call, the page was 
		// a replacement candidate, but is no longer a candidate. 
		// If the page is not in the pool, 
		// choose a frame (from the set of replacement candidates) 
		// to hold this page, read the page (using the appropriate DB 
		// class method) and pin it. 
		// (If there is no candidate, return error.)
		// Also, you must write out the old page in chosen 
		// frame if it is dirty before reading new page. 
		// you must check on read/write mode, if the pinned page
		// has been pinned before, make sure that write 
		// exclusive condition is hold, else return error

		Status unpinPage(PageId globalPageId_in_a_DB, int dirty, int hate);
		// hate should be TRUE if type page is “hated” and d.
		// User should call this with dirty = TRUE if the page
		// has be modified.
		// If so, this call should set the dirty bit for this frame.
		// Further, if pin_count > 0, should decrement it. 
		// If pin_count = 0 before this call, return error.

		Status newPage(PageId& firstPageId, Page*& firstpage, int howmany = 1);
		// Find a frame in the buffer pool for the first page. 
		// If a frame exists, call DB object to allocate a run 
		// of new pages and pin it in read_write_mode. 
		// (This call allows a client of the Buffer Manager 
		// to allocate pages on disk.) 
		// If buffer is full, i.e., you can’t find a frame 
		// for the first page, return error. 
		
		Status freePage(PageId globalPageId);
		// user should call this method if it needs to delete a page
		// this routine will call DB to deallocate the page 
		// (When the page is be pinned you should return error.)

		Status flushPage(PageId pageid);
		// Used to flush a particular page of the buffer pool to disk
		// (without modify love/hate list)
		// Should call the write_page method of the DB class
		
		Status flushAllPages();
		// Flush all pages of the buffer pool to disk, as per flushPage.

		Page* getPage(PageId pageid);
		int getFrame(PageId pageid);
		int getFreeFrame();

		// DO NOT REMOVE THESE METHODS ================================================
		// For backward compatibility with lib
		Status pinPage(PageId PageId_in_a_DB, Page*& page, int emptyPage = 0);
		Status unpinPage(PageId globalPageId_in_a_DB, int dirty = FALSE) {
			return unpinPage(globalPageId_in_a_DB, dirty, FALSE);
		}
		// ===========================================================================
};

#endif
