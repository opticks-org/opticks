/*
 * The information in this file is
 * Copyright(c) 2020 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "testPlugin.h"
using namespace std;


COMET_tests::COMET_tests():
    mNumPluginArgs( 1 )
{
    mbAbort = false;
    mbError = false;
    interactiveFlag = false;

    // init data members inherited from AlgorithmShell
    setName( "Test Plugin" );
    setVersion( "1.0" );
    setCreator( "Adam Ewing" );
    setCopyright( "None" );
    setShortDescription( "This is a dummy plugin that does nothing." );
    setDescription( "This is a dummy plugin that does nothing." );
    setType( "Algorithm" );
    setMenuLocation( "[Spectral]\\Process\\Test PlugIn" );	
}


COMET_tests::~COMET_tests() {}


bool COMET_tests::isInputValid( PlugInArgList * argList )
{
    return true;
}

bool COMET_tests::getInputSpecification( PlugInArgList * &argList )
{
	return true;
}

bool COMET_tests::getOutputSpecification( PlugInArgList * &argList )
{
    return true;
}

bool COMET_tests::getInterfacePointers()
{
    return true;
}

bool COMET_tests::execute( PlugInArgList *inputArgList, PlugInArgList *outputArgList )
{
	return true;
}
