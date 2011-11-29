/*****************************************************************************/
/*************** Implementation of the Buffer Manager Layer ******************/
/*****************************************************************************/


#include "buf.h"

// Define buffer manager error messages

static const char* bufErrMsgs[] = { 
    // todo: error message strings go here
    "HASTERROR: Hash table error.",
    "HASHRMERROR: Remove error, not found in hash table.",
    "BUFFEREXCEEDED: Buffer pool is full.",
    "PMODEERROR: Page is busy, can't be pinned.",
    "PAGENOTFOUND: Page isn't in pool.",
    "PINCOUNTERROR: Pin count is bad.",
    "NOAVAILPOOL: No available pool.",
    "PAGEISBUSY: Page is busy, can't be free."
};

// Create a static "error_string_table" object and register the error messages
// with minibase system 

static error_string_table bufTable(BUFMGR,bufErrMsgs);


// todo: Design your own hash table and replacement policy here.
void HashTable::insert(PageId i_page_no, FrameId i_frame_no) {
    int pos = i_page_no % HTSIZE;
    hash_table[pos].push_back(make_pair(i_page_no, i_frame_no));
}

FrameId HashTable::getFrameId(PageId q_page_no) {
    int pos = q_page_no % HTSIZE;
    list<pair<PageId, FrameId> >::const_iterator iter;
    for (iter = hash_table[pos].begin(); iter != hash_table[pos].end(); iter++) {
        if (iter->first == q_page_no) {
            return iter->second;
        }
    }
    return -1;
}

bool HashTable::remove(PageId r_page_no) {
    if (r_page_no < 0)
        return false;
    int pos = r_page_no % HTSIZE;
    list<pair<PageId, FrameId> >::iterator iter;
    for (iter = hash_table[pos].begin(); iter != hash_table[pos].end(); iter++) {
        if (iter->first == r_page_no) {
            hash_table[pos].erase(iter);
            return true;
        }
    }
    return false;
}

FrameId BufMgr::getFreeFrameId() {
    for (int i = 0; i < buf_number; i++) {
        if (bufDescr[i].free) {
            return i;
        }
    }
    return -1;
}

// Replacement policy
FrameId BufMgr::getAvailableFrameId() {
    // Find if there are free pools.
    int fn = getFreeFrameId();
    if (fn != -1)
        return fn;

    // Replacement, "love conquer hate" is in unpinPage.
    if (!hate_list.empty()) { // First replace the item in hate list
        fn = hate_list.front();
        hate_list.pop_front();
        return fn;
    } else if (!love_list.empty()) { // If hate list has nothing, replace lovelist.
        fn = love_list.back();
        love_list.pop_back();
        return fn;
    }
    // Pool is full, and no item can be replace.
    return -1;
}

// **********************************************************
// Class Buffer Manager
// **********************************************************

// **********************************************************
// BufMgr class constructor
BufMgr::BufMgr(int numbuf, Replacer* replacer) {
    // DO NOT REMOVE THESE TWO LINE ==============================
    minibase_globals->DummyBufMgr=this;
    replacer = 0;						// DISREGARD THE PARAMETER
    // ===========================================================

    // todo: fill the body
    buf_number = free_buf_n = numbuf;
    bufPool = new Page[numbuf];
    bufDescr = new FrameDescr[numbuf];
}

// **********************************************************
// BufMgr class destructor
BufMgr::~BufMgr() {
    // todo: fill the body
    delete[] bufPool;
    delete[] bufDescr;
    bufPool = NULL;
    bufDescr = NULL;
}

Status BufMgr::pinPage(PageId PageId_in_a_DB, Page*& page, MODE mode) {
    // todo: fill the body
    FrameId frame_no = hash_table.getFrameId(PageId_in_a_DB);

    // not in pool
    if (frame_no == -1) { 
        frame_no = getAvailableFrameId();

        // pool is full
        if (frame_no == -1) 
            return MINIBASE_FIRST_ERROR(BUFMGR, NOAVAILPOOL);

        // write back if old page is dirty
        if (bufDescr[frame_no].pin_count > 0 && bufDescr[frame_no].dirty) {
            Status st = MINIBASE_DB->write_page(bufDescr[frame_no].page_no, &bufPool[frame_no]);
            if (st != OK)
                return MINIBASE_CHAIN_ERROR(BUFMGR, st);
        }

        // add to hash table
        hash_table.remove(bufDescr[frame_no].page_no);
        hash_table.insert(PageId_in_a_DB, frame_no);

        // read into buffer
        Status rst = MINIBASE_DB->read_page(PageId_in_a_DB, &bufPool[frame_no]);
        if (rst != OK)
            return MINIBASE_CHAIN_ERROR(BUFMGR, rst);

        bufDescr[frame_no].page_no = PageId_in_a_DB;
    } else {
        if (bufDescr[frame_no].pin_count > 0) { // have been pinned
            if (bufDescr[frame_no].mode == READ_WRITE_MODE || mode == READ_WRITE_MODE)
                return MINIBASE_FIRST_ERROR(BUFMGR, PINMODEERROR);
        } else if (bufDescr[frame_no].pin_count == 0) { // it's possible to be in the l/h list.
            hate_list.remove(frame_no);
            love_list.remove(frame_no);
        }
    }
    bufDescr[frame_no].pin_count++;
    bufDescr[frame_no].mode = mode;
    bufDescr[frame_no].dirty = false;
    bufDescr[frame_no].free = false;
    page = &bufPool[frame_no];
    return OK;	
}

// **********************************************************
Status BufMgr::unpinPage(PageId globalPageId_in_a_DB, int dirty=FALSE, int hate=FALSE) {
    // todo: fill the body
    FrameId frame_no = hash_table.getFrameId(globalPageId_in_a_DB);
    if (frame_no == -1)
        return MINIBASE_FIRST_ERROR(BUFMGR, PAGENOTFOUND);

    if (bufDescr[frame_no].pin_count == 0)
        return MINIBASE_FIRST_ERROR(BUFMGR, PINCOUNTERROR);

    // write back if dirty
    if (dirty) {
        Status st = MINIBASE_DB->write_page(bufDescr[frame_no].page_no, &bufPool[frame_no]);
        if (st != OK)
            return MINIBASE_CHAIN_ERROR(BUFMGR, st);
    }

    bufDescr[frame_no].dirty = dirty;
    bufDescr[frame_no].pin_count--;
    if (bufDescr[frame_no].pin_count == 0) {
        //hash_table.remove(globalPageId_in_a_DB);
    } else if (bufDescr[frame_no].pin_count > 0) {
        return OK;
    } else {
        cerr << "QQQQQQQ\n";
    }

    if (hate) {
        bool pin_love = false;
        // When someone unpin a love page with hate, you should not update the time.
        for (list<FrameId>::iterator iter = love_list.begin(); iter != love_list.end(); iter++) {
            if (*iter == frame_no) {
                pin_love = true;
                break;
            }
        }
        if (!pin_love) {
            hate_list.remove(frame_no);
            hate_list.push_front(frame_no);
        }
    } else {
        // Check if hate list has had frame_no, if has, remove it. (Love conquer hate)
        for (list<FrameId>::iterator iter = hate_list.begin(); iter != hate_list.end(); iter++) {
            if (*iter == frame_no) {
                hate_list.remove(frame_no);
                break;
            }
        }
        love_list.remove(frame_no);
        love_list.push_front(frame_no);
    }
    return OK;
}

// **********************************************************
Status BufMgr::newPage(PageId& firstPageId, Page*& firstPage, int howmany) {
    // DO NOT REMOVE THIS LINE =========================
    howmany = 1;
    // ================================================
    // todo: fill the body
    FrameId frame_no = getFreeFrameId();
    if (frame_no == -1)
        return MINIBASE_FIRST_ERROR(BUFMGR, BUFFEREXCEEDED);

    Status ast = MINIBASE_DB->allocate_page(firstPageId, howmany);
    if (ast != OK)
        return MINIBASE_CHAIN_ERROR(BUFMGR, ast);

    hash_table.insert(firstPageId, frame_no);
    bufDescr[frame_no].page_no = firstPageId;
    bufDescr[frame_no].pin_count = 1;
    bufDescr[frame_no].mode = READ_WRITE_MODE;
    bufDescr[frame_no].dirty = false;
    bufDescr[frame_no].free = false;
    firstPage = &bufPool[frame_no];
    return OK;
}

// **********************************************************
Status BufMgr::freePage(PageId globalPageId) {
    // todo: fill the body
    FrameId frame_no = hash_table.getFrameId(globalPageId);
    if (frame_no != -1 && bufDescr[frame_no].pin_count > 0)
        return MINIBASE_FIRST_ERROR(BUFMGR, PAGEISBUSY);
    
    Status dst = MINIBASE_DB->deallocate_page(globalPageId);
    if (dst != OK)
        return MINIBASE_CHAIN_ERROR(BUFMGR, dst);
    
    bufDescr[frame_no].clear();
    hash_table.remove(globalPageId);
    return OK;
}

// **********************************************************
Status BufMgr::flushPage(PageId pageId) {
    // todo: fill the body
    FrameId frame_no = hash_table.getFrameId(pageId);
    if (frame_no == -1)
        return MINIBASE_FIRST_ERROR(BUFMGR, PAGENOTFOUND);
    Status wst = MINIBASE_DB->write_page(pageId, &bufPool[frame_no]);
    if (wst != OK)
        return MINIBASE_CHAIN_ERROR(BUFMGR, wst);
    return OK;
}

// **********************************************************
Status BufMgr::flushAllPages() {
    // todo: fill the body
    for (int i = 0; i < buf_number; i++) {
        if (bufDescr[i].pin_count > 0)
            flushPage(bufDescr[i].page_no);
    }
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
