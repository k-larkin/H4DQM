#include "interface/Event.hpp"

void Event::createOutBranches (TTree* tree,treeStructData& treeData)
{
  //Instantiate the tree branches
  tree->Branch("evtNumber",&treeData.evtNumber,"evtNumber/i");
  tree->Branch("evtTimeDist",&treeData.evtTimeDist,"evtTimeDist/i");
  tree->Branch("evtTimeStart",&treeData.evtTimeStart,"evtTimeStart/i");
  tree->Branch("evtTime",&treeData.evtTime,"evtTime/i");

  tree->Branch("boardTriggerBit",&treeData.boardTriggerBit,"boardTriggerBit/i");

  tree->Branch("triggerWord",&treeData.triggerWord,"triggerWord/i");

  tree->Branch("nAdcChannels",&treeData.nAdcChannels,"nAdcChannels/i");
  tree->Branch("adcBoard",treeData.adcBoard,"adcBoard[nAdcChannels]/i");
  tree->Branch("adcChannel",treeData.adcChannel,"adcChannel[nAdcChannels]/i");
  tree->Branch("adcData",treeData.adcData,"adcData[nAdcChannels]/i");

  tree->Branch("nTdcChannels",&treeData.nTdcChannels,"nTdcChannels/i");
  tree->Branch("tdcBoard",treeData.tdcBoard,"tdcBoard[nTdcChannels]/i");
  tree->Branch("tdcChannel",treeData.tdcChannel,"tdcChannel[nTdcChannels]/i");
  tree->Branch("tdcData",treeData.tdcData,"tdcData[nTdcChannels]/i");

  tree->Branch("nDigiSamples",&treeData.nDigiSamples,"nDigiSamples/i");
  tree->Branch("digiGroup",treeData.digiGroup,"digiGroup[nDigiSamples]/i");
  tree->Branch("digiChannel",treeData.digiChannel,"digiChannel[nDigiSamples]/i");
  tree->Branch("digiSampleIndex",treeData.digiSampleIndex,"digiSampleIndex[nDigiSamples]/i");
  tree->Branch("digiSampleValue",treeData.digiSampleValue,"digiSample[nDigiSamples]/F");

  return ;
} 


// ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----


void Event::fillTreeData (treeStructData & treeData)
{
  treeData.evtNumber = evtNumber ;
  //  if (DEBUG_UNPACKER) printf (" =  =  =  =  =  =  FILLING EVENT %d  =  =  =  =  = \n", treeData.evtNumber) ;
  treeData.evtTime = evtTime ;
  treeData.evtTimeStart = evtTimeStart ;
  treeData.evtTimeDist = evtTimeDist ;

  treeData.boardTriggerBit = boardTriggerBit ;

  treeData.triggerWord = 0 ;
  for (unsigned int i = 0 ; i<fmin (32, triggerWord.size ()) ;++i)
    treeData.triggerWord += triggerWord[i]>>i ;

  treeData.nAdcChannels = adcValues.size () ;
  //  if (DEBUG_UNPACKER) printf ("FILLING %d ADC values\n", treeData.nAdcChannels) ;
  for (unsigned int i = 0 ;i<fmin (MAX_ADC_CHANNELS, adcValues.size ()) ;++i)
    {
      treeData.adcBoard[i] = adcValues[i].board ;
      treeData.adcChannel[i] = adcValues[i].channel ;
      treeData.adcData[i] = adcValues[i].adcReadout ;
    }

  treeData.nTdcChannels = tdcValues.size () ;
  //  if (DEBUG_UNPACKER) printf ("FILLING %d TDC values\n", treeData.nTdcChannels) ;
  for (unsigned int i = 0 ;i<fmin (MAX_TDC_CHANNELS, tdcValues.size ()) ;++i)
    {
      treeData.tdcBoard[i] = tdcValues[i].board ;
      treeData.tdcChannel[i] = tdcValues[i].channel ;
      treeData.tdcData[i] = tdcValues[i].tdcReadout ;
    }

  treeData.nDigiSamples = digiValues.size () ;
  //  if (DEBUG_UNPACKER) printf ("FILLING %d DIGI values\n", treeData.nDigiSamples) ;
  for (unsigned int i = 0 ;i<fmin (MAX_DIGI_SAMPLES, digiValues.size ()) ;++i)
    {
      treeData.digiGroup[i] = digiValues[i].group ;
      treeData.digiChannel[i] = digiValues[i].channel ;
      treeData.digiSampleIndex[i] = digiValues[i].sampleIndex ;
      treeData.digiSampleValue[i] = digiValues[i].sampleValue ;
    }
  return ;
}


// ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----


void Event::Fill ()
{
  fillTreeData (thisTreeEvent_) ;
  outTree_->Fill () ;
  return ;
}


// ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----


int Event::clear ()
{
  evtNumber = -1 ;
  boardTriggerBit = 0 ;
  triggerWord.clear () ;
  adcValues.clear () ; 
  tdcValues.clear () ; 
  digiValues.clear () ; 
  evtTimeDist = -1 ;
  evtTimeStart = -1 ;
  evtTime = -1 ;
}