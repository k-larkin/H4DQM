#include "interface/SpillUnpack.hpp"

WORD spillHeaderValue;
WORD spillTrailerValue;
WORD eventHeaderValue;
WORD eventTrailerValue;
WORD boardHeaderValue;
WORD boardTrailerValue;


// ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----


SpillUnpack::SpillUnpack(ifstream* in, TFile* out, TTree * outTree) {
  rawFile = in ;
  outFile_ = out ;
  outTree_ = outTree ;

  spillHeaderValue = *((uint32_t*)"SPLH");
  spillTrailerValue = *((uint32_t*)"SPLT");
  eventHeaderValue = *((uint32_t*)"EVTH");
  eventTrailerValue = *((uint32_t*)"EVNT");
  boardHeaderValue = *((uint32_t*)"BRDH");
  boardTrailerValue = *((uint32_t*)"BRDT");

  event_ = new Event (outFile_, outTree) ;

}


// ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----


SpillUnpack::~SpillUnpack ()
{
  delete event_ ;
}


// ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----


int SpillUnpack::Unpack(int events = -1){

  WORD word;
  //  InitBoards () ;

  while (!rawFile->eof()) {
      spillHeader spillH;

      rawFile->read ((char*)&word, WORDSIZE);
      
      cout << "[SpillUnpack][Unpack]      | reading word " << word << endl;
      
      if (word==spillHeaderValue) {
        rawFile->read ((char*)&spillH.runNumber, WORDSIZE);
        rawFile->read ((char*)&spillH.spillNumber, WORDSIZE);
        rawFile->read ((char*)&spillH.spillSize, WORDSIZE);
        rawFile->read ((char*)&spillH.nEvents, WORDSIZE);
      
        if (DEBUG_UNPACKER) {
          cout << "[SpillUnpack][Unpack]      | ======= BEGIN SPILL ======= \n" ;
          cout << "[SpillUnpack][Unpack]      | Spill " << spillH.spillNumber << "\n" ;
          cout << "[SpillUnpack][Unpack]      | Events in spill " << spillH.nEvents << "\n" ;
          cout << "[SpillUnpack][Unpack]      | unpacking " << events << " events" << endl ;
        }
        
        if (-1 == events) events = spillH.nEvents ; 
        UnpackEvents (events) ;
      } 
      else 
      { 
        cout << "[SpillUnpack][Unpack]      | ERROR corrupted RAW file. "
             << "Expecting spill header, read " << word << endl ;
        return 1 ;
      }
  }

}


// ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----


void SpillUnpack::InitBoards(){
  //PG FIXME do we want to init only existing boards?

  if (DEBUG_VERBOSE_UNPACKER) cout << "[SpillUnpack][InitBoards]  |\n" ;

  //init differend kind of boards
  for (int c = 0 ; c < (BoardTypes_t)_MAXBOARDTYPE_ ; ++c) {

    BoardTypes_t i = static_cast<BoardTypes_t> ( c ) ;

    switch(i){
    case _TIME_:
      boards_[i] = new TIME;
      break;
    case _CAENVX718_:
      boards_[i]= new CAEN_VX718;
      break;
    case _CAENV1742_:
      boards_[i]= new CAEN_V1742 (/* PG FIXME number of words */);
      break;
    case _CAENV513_:
      boards_[i]= new CAEN_V513;
      break;
    case _CAENV262_:
      boards_[i]= new CAEN_V262;
      break;
    case _CAENV792_:
      boards_[i]= new CAEN_V792;
      break;
    case _CAENV1290_:
      boards_[i]= new CAEN_V1290;
      break;
    case _CAENV1495PU_:
      boards_[i]= new CAEN_V1495PU;
      break;
    case _CAENV560_:
      boards_[i]= new CAEN_V560;
      break;
    case _UNKWN_:
      //TO DO decide what to do.continue?
      break;
    } // switch
  } // for
}


// ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----


void SpillUnpack::UnpackEvents (int nevents) {

  WORD word;
  WORD nevt=0;

  // loop over the input file
  while (!rawFile->eof()) {
    
    if (nevt > nevents) break ;

    eventHeader eventH;

    rawFile->read ((char*)&word, WORDSIZE);
    if (word == eventHeaderValue) {
      if (DEBUG_VERBOSE_UNPACKER) {
          cout << "[SpillUnpack][UnpackEvents]| reading event header\n" ;
        }

      nevt++;
      rawFile->read ((char*)&eventH.eventNumber, WORDSIZE);
      rawFile->read ((char*)&eventH.eventSize, WORDSIZE);
      rawFile->read ((char*)&eventH.nBoards, WORDSIZE);

      event_->evtNumber = eventH.eventNumber ;

      if (DEBUG_UNPACKER) {
        cout << "[SpillUnpack][UnpackEvents]| ======== EVENT START ======= " << endl;
        cout << "[SpillUnpack][UnpackEvents]|  Event " << eventH.eventNumber << endl;
        cout << "[SpillUnpack][UnpackEvents]|  Boards in event " << eventH.nBoards << endl;
      }
      
      if (eventH.eventNumber!=nevt) {
        cout << "[SpillUnpack][UnpackEvents]| ERROR event numbering inconsistent! " << endl ;
      }
      
      UnpackBoards (eventH.nBoards) ;

      event_->Fill () ;

      continue;
    } 
      
    if (word==eventTrailerValue) {
      if (DEBUG_VERBOSE_UNPACKER) {
          cout << "[SpillUnpack][UnpackEvents]| reading event trailer\n" ;
        }
      if (DEBUG_UNPACKER) cout << "[SpillUnpack][UnpackEvents]|  ========= EVENT END ======== \n" ;
      continue;
    } 
    
    if (word==spillTrailerValue) {
      if (DEBUG_VERBOSE_UNPACKER) {
          cout << "[SpillUnpack][UnpackEvents]| reading spill trailer\n" ;
        }
      if (DEBUG_UNPACKER) cout << "[SpillUnpack][UnpackEvents]| ========= SPILL END ======== \n" ;
      if (DEBUG_UNPACKER || nevents!=nevt) cout << "[SpillUnpack][UnpackEvents]| Read " 
                                                << nevt << " events. Expected " 
                                                << nevents << "\n" ;
      break;
    }

//    cout << " ERROR corrupt RAW file. Read " << word << endl;

  } // loop over the input file    
}


// ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----


int SpillUnpack::AddBoard (boardHeader bH)
{
  WORD boardType = GetBoardTypeId (bH.boardID) ;
  
  if (DEBUG_VERBOSE_UNPACKER) {
    cout << "[SpillUnpack][AddBoard]    | Creating new board type " << boardType << endl;
  }
  
  switch(boardType){
  case _TIME_:
    boards_[bH.boardID] = new TIME;
    break;
  case _CAENVX718_:
    boards_[bH.boardID]= new CAEN_VX718;
    break;
  case _CAENV1742_:
    boards_[bH.boardID]= new CAEN_V1742 (bH.boardSize) ;
    break;
  case _CAENV513_:
    boards_[bH.boardID]= new CAEN_V513;
    break;
  case _CAENV262_:
    boards_[bH.boardID]= new CAEN_V262;
    break;
  case _CAENV792_:
    boards_[bH.boardID]= new CAEN_V792;
    break;
  case _CAENV1290_:
    boards_[bH.boardID]= new CAEN_V1290;
    break;
  case _CAENV1495PU_:
    boards_[bH.boardID]= new CAEN_V1495PU;
    break;
  case _CAENV560_:
    boards_[bH.boardID]= new CAEN_V560;
    break;
  case _UNKWN_:
    boards_[bH.boardID]= new DummyBoard;
    //TO DO decide what to do.continue?
    break;
  }
  return boards_.size () ;
} 


// ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----


WORD SpillUnpack::GetBoardTypeId (WORD compactId)
{
  WORD myResult= (compactId & static_cast<WORD>(0xFF000000))>>24;
  return myResult;
}


// ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----


void SpillUnpack::UnpackBoards(int nboards) {

  WORD word ;
  WORD nbrd = 0 ;

  // loop on boards to be read
  for (int iBoard = 0 ; iBoard < nboards ; ++iBoard) {

    boardHeader bH;

    rawFile->read ((char*)&word, WORDSIZE);
    if (word == boardHeaderValue) {
      nbrd++;
      rawFile->read ((char*)&bH.boardID, WORDSIZE);
      rawFile->read ((char*)&bH.boardSize, WORDSIZE);
      int boardType = GetBoardTypeId (bH.boardID) ;

      if (DEBUG_UNPACKER) {
        cout << "[SpillUnpack][UnpackBoards]| ======== BOARD START ======= \n" ;
        cout << "[SpillUnpack][UnpackBoards]| Board " << nbrd << "/" << nboards
             << "  ID " << bH.boardID
             << "  type " << boardType
             << "  size " << bH.boardSize << "\n" ;
      }

      if (boards_.find (bH.boardID) == boards_.end ()) // not found the board crea una nuova
        {
          AddBoard (bH) ;
        }
      //PG unpack the board boards_[bH.boardID]
      if (boards_[bH.boardID]) boards_[bH.boardID]->Unpack (*rawFile, event_) ;
        else cout << "[SpillUnpack][UnpackBoards]| [ERROR] Unknown board ID " << bH.boardID << endl;

      continue;
    } 

    if (word == eventTrailerValue) {
      if (DEBUG_UNPACKER) cout << "[SpillUnpack][UnpackBoards]| ========= EVENT END ======== \n" ;
      if (DEBUG_UNPACKER || nboards!=nbrd) 
         cout << "[SpillUnpack][UnpackBoards]| [ERROR] Read " 
              << nbrd << " boards. Expected " 
              << nboards << "\n" ;
      break;
    }

    if (word==boardTrailerValue) {
      if (DEBUG_UNPACKER) cout << "[SpillUnpack][UnpackBoards]| ========= BOARD END ======== " << endl;
      continue;
    } 
          
    if (DEBUG_VERBOSE_UNPACKER) { 
        cout << "[SpillUnpack][UnpackBoards]| Something went wrong, read word " << word << endl;
      }
  }
}
