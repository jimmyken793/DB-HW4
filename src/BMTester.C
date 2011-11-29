// ** comment by cclee 2011/11/15
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <iostream>
using namespace std;
#include <assert.h>
#include <unistd.h>

#include "buf.h"
#include "db.h"
#include <pwd.h>


#include "BMTester.h"

//extern "C" int getpid();
//extern "C" int unlink( const char* );


BMTester::BMTester() : TestDriver( "buftest" )
{}


BMTester::~BMTester()
{}

//----------------------------------------------------
// test 1
//      Testing pinPage, unpinPage, and whether a dirty page 
//      is written to disk
//----------------------------------------------------

int BMTester::test1() 
{
	Status st;
	Page*	pg;
	int	first,last;
	char data[200]; 
	first = 5;
	last = first + NUMBUF + 5;

	cout << "--------------------- Test 1 ----------------------\n";
        st = OK;
	for (int i=first;i<=last;i++){
		// ** pin page 
		if (MINIBASE_BM->pinPage(i,pg,READ_WRITE_MODE)!=OK)  {
       		        st = FAIL;
        	}
        	cout<<"after pinPage" << i <<endl;
		// ** write data into page
		sprintf(data,"This is test 1 for page %d\n",i);
		strcpy((char*)pg,data);
		// ** unpin page
		if (MINIBASE_BM->unpinPage(i,1)!=OK) {
            		st = FAIL;
        	}
        	cout<<"after unpinPage"<< i << endl;
    	}

	cout << "\n" << endl;

    	for (int i=first;i<=last;i++){
		// ** pin page again
       		if (MINIBASE_BM->pinPage(i,pg,0)!=OK) {
            		st = FAIL;
        	}
        	cout<<"PAGE["<<i<<"]: "<<(char *)pg;
		// ** check data be wrote is right
        	sprintf(data,"This is test 1 for page %d\n",i);
        	if (strcmp(data,(char*)pg)) {
            		st = FAIL;
            		cout << "Error: page content incorrect!\n";
        	}
        	if (MINIBASE_BM->unpinPage(i)!=OK)  {
            		st = FAIL;
        	}
        }
	if(st != OK)
		MINIBASE_SHOW_ERRORS();
	minibase_errors.clear_errors();
	return st == OK;
}

//-----------------------------------------------------------
// test 2
// Testing read/write mode
//------------------------------------------------------------

int BMTester::test2()
{
	Status st;
	Page*	pg;
	int	first,last;
	first = 5;
	last = 10;

	cout << "--------------------- Test 2 ----------------------\n";
   st = OK;
	MODE mode;
	// ** pin page with read or read_write mode
	for (int i=first;i<=last;i++) {
		if (i % 2 == 0) {
			mode = READ_MODE;
		} else {
			mode = READ_WRITE_MODE;	  
		}
		if (MINIBASE_BM->pinPage(i,pg,mode)!=OK)  {
   			st = FAIL;
  		}
     	cout<<"after pinPage" << i <<endl;
	}
	// ** pin 2 page with read mode
	if (MINIBASE_BM->pinPage(first,pg,READ_MODE) == OK) {
     		cout << "Error: pin page " << first << " incorrect!\n";
		st = FAIL;
	}
     	cout<<"after pinPage" << first <<endl;
	if (MINIBASE_BM->pinPage(first + 1,pg,READ_MODE) != OK) {
		st = FAIL;
     		cout << "Error: pin page " << first + 1 << " incorrect!\n";
	}
	// ** pin other page with read or read/write mode
     	cout<<"after pinPage" << first + 1 <<endl;
	for (int i=first + 2;i<=last;i++) {
		if (i % 2 == 1) {
			mode = READ_MODE;
		} else {
			mode = READ_WRITE_MODE;	  
		}
		if (MINIBASE_BM->pinPage(i,pg,mode) == OK)  {
	     		cout << "Error: pin page " << i << " incorrect!\n";
	   		st = FAIL;
	  	}
     	cout<<"after pinPage" << i <<endl;
	}
	for (int i=first;i<=last;i++) {
		if (MINIBASE_BM->unpinPage(i, false, false)!=OK)  {
   			st = FAIL;
  		}
     	cout<<"after unpinPage" << i <<endl;
	}
	if(st != OK)
		MINIBASE_SHOW_ERRORS();
	minibase_errors.clear_errors();
	return st == OK;
}

//---------------------------------------------------------
// test 3
//      Testing  newPage,pinPage, freePage, error protocol
//---------------------------------------------------------


int BMTester::test3() 
{
  Status st;
  int pages[30];
  Page* pagesptrs[30];
  Page* pgptr;
  int i;
  
  cout << "--------------------- Test 3 ----------------------\n";
  st = OK;
  // Allocate 10 pages from database
  for ( i = 0; i < 10; i++)
    {
      if (MINIBASE_BM->newPage(pages[i], pagesptrs[i]) !=OK) {
        st = FAIL;
	cout << "\tnewPage failed...\n";
      }
     cout<<"after pinPage" << i <<endl;
    }
  // try to pin read_write page again
   for (i = 0; i < 5; i++)
    {
      cout << "pin page " << i << " " << pages[i] << endl;
      if (MINIBASE_BM->pinPage(pages[i], pgptr,READ_MODE) == OK) {
        st = FAIL;
	cout << "Error: pin a read write page again\n";
      }
   }

  // Try to free pinned pages
  for (i = 5; i < 10; i++)
    {
      cout << "free page " << pages[i] << endl;
      if (MINIBASE_BM->freePage(pages[i]) == OK) {
        st = FAIL;
	cout << "Error: pinned page freed!\n";
      }
    }
  // ** unpin all page
 for (i = 0; i < 10; i++)
  {
   
    if (MINIBASE_BM->unpinPage(pages[i]) !=OK) {
      st = FAIL;
      cout << "Error: unpin page fail.";
    }
    cout << "unpin page " << pages[i] << endl;
  }


 

  // ** Pin 10 pages twice
  for (i = 0; i < 10; i++)
    {
      cout << "pin page " << i << " " << pages[i] << endl;
      if (MINIBASE_BM->pinPage(pages[i], pgptr)!= OK) {
        st = FAIL;
	cout << "Error: pinPage failed...\n";
      }

      cout << "pin page " << i << " " << pages[i] << endl;
      if (MINIBASE_BM->pinPage(pages[i], pgptr)!= OK) {
        st = FAIL;
	cout << "Error: pinPage failed...\n";
      }
      if (pgptr != pagesptrs[i]) {
        st = FAIL;
	cout << "Error: Pinning error in a second time ...\n";
      }
    }


  // Now free page 0 thru 9 by first unpinning each page twice


  for (i = 0; i < 10; i++)
  {
   
    if (MINIBASE_BM->unpinPage(pages[i]) !=OK) {
      st = FAIL;
      cout << "Error: unpin page failed.";
    }
    if (MINIBASE_BM->unpinPage(pages[i]) !=OK) {
      st = FAIL;
      cout << "Error: unpin page failed.";
    }
    if (MINIBASE_BM->freePage(pages[i]) !=OK) {
      st = FAIL;
      cout << "Error: free page failed.";
    }
    cout << "unpin and free  page " << pages[i] << endl;
  }

  // Get 14 more pages
  for (i = 10; i < 24; i++)
  {
    if(MINIBASE_BM->newPage(pages[i], pagesptrs[i])!=OK) {
      st = FAIL;
      cout << "Error: new page error";
    }
     cout << "new  page " << i << "," << pages[i] << endl;
  }
  if(st != OK)
      MINIBASE_SHOW_ERRORS();

  minibase_errors.clear_errors();
  return st == OK;
}




//-------------------------------------------------------------
// test 4
//-------------------------------------------------------------

int BMTester::test4(){
  return TRUE;
}

//-------------------------------------------------------------
// test 5
//-------------------------------------------------------------

int BMTester::test5(){
  return TRUE;
}

//----------------------------------------------------------
// Test 6
//-----------------------------------------------------------

int BMTester::test6()
{
  return TRUE;
}

const char* BMTester::testName()
{
    return "Buffer Management";
}


void BMTester::runTest( Status& status, TestDriver::testFunction test )
{
    // build a new db -- by cclee
    minibase_globals = new SystemDefs( status, dbpath, logpath, 
				  NUMBUF+50, 500, NUMBUF, "Clock" );
    if ( status == OK )
      {
        TestDriver::runTest(status,test);
		// marked by Neil.
        // delete minibase_globals; 
		minibase_globals = 0;
      }
    // renew dbpath, logpath -- by cclee
    char* newdbpath;
    char* newlogpath;
    char remove_logcmd[50];
    char remove_dbcmd[50];

    newdbpath = new char[ strlen(dbpath) + 20];
    newlogpath = new char[ strlen(logpath) + 20];
    strcpy(newdbpath,dbpath);
    strcpy(newlogpath, logpath);

    sprintf(remove_logcmd, "/bin/rm -rf %s", logpath);
    sprintf(remove_dbcmd, "/bin/rm -rf %s", dbpath);
    system(remove_logcmd);
    system(remove_dbcmd);
    sprintf(newdbpath, "%s", dbpath);
    sprintf(newlogpath, "%s", logpath);


    unlink( newdbpath );
    unlink( newlogpath );

    delete newdbpath; delete newlogpath;

}


Status BMTester::runTests()
{
    return TestDriver::runTests();
}


Status BMTester::runAllTests()
{
    return TestDriver::runAllTests();
}
