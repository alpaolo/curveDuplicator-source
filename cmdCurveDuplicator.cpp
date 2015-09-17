/////////////////////////////////////////////////////////////////////////////
// cmdCurveDuplicator.cpp : command file
//

#include "StdAfx.h"
#include "CurveDuplicatorPlugIn.h"

////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
//
// BEGIN CurveDuplicator command
//

#pragma region CurveDuplicator command


// Do NOT put the definition of class CCommandCurveDuplicator in a header
// file.  There is only ONE instance of a CCommandCurveDuplicator class
// and that instance is the static theCurveDuplicatorCommand that appears
// immediately below the class definition.

class CCommandCurveDuplicator : public CRhinoCommand
{
public:
	// The one and only instance of CCommandCurveDuplicator is created below.
	// No copy constructor or operator= is required.  Values of
	// member variables persist for the duration of the application.

	// CCommandCurveDuplicator::CCommandCurveDuplicator()
	// is called exactly once when static theCurveDuplicatorCommand is created.
	CCommandCurveDuplicator() {}

	// CCommandCurveDuplicator::~CCommandCurveDuplicator()
	// is called exactly once when static theCurveDuplicatorCommand is
	// destroyed.  The destructor should not make any calls to
	// the Rhino SDK.  If your command has persistent settings,
	// then override CRhinoCommand::SaveProfile and CRhinoCommand::LoadProfile.
	~CCommandCurveDuplicator() {}

	// Returns a unique UUID for this command.
	// If you try to use an id that is already being used, then
	// your command will not work.  Use GUIDGEN.EXE to make unique UUID.
	UUID CommandUUID()
	{
		// {36F10406-361B-4846-8F2F-D618AE48ECC8}
		static const GUID CurveDuplicatorCommand_UUID =
		{ 0x36F10406, 0x361B, 0x4846, { 0x8F, 0x2F, 0xD6, 0x18, 0xAE, 0x48, 0xEC, 0xC8 } };
		return CurveDuplicatorCommand_UUID;
	}

	// Returns the English command name.
	const wchar_t* EnglishCommandName() { return L"CurveDuplicator"; }

	// Returns the localized command name.
	const wchar_t* LocalCommandName() const { return L"CurveDuplicator"; }

	// Rhino calls RunCommand to run the command.
	CRhinoCommand::result RunCommand( const CRhinoCommandContext& );
};

// The one and only CCommandCurveDuplicator object.  
// Do NOT create any other instance of a CCommandCurveDuplicator class.
static class CCommandCurveDuplicator theCurveDuplicatorCommand;

CRhinoCommand::result CCommandCurveDuplicator::RunCommand( const CRhinoCommandContext& context )
{


	/*********************************************************************************

	*********************************************************************************/

	ON_wString wStr;
	int getObjectCount;
	const ON_Curve* masterCurve;
	const ON_Curve* curveToDuplicate;
	const CRhinoObject* objToDuplicate;


	/****************************** GET CURVE AND PROMPTS ************************************/
	CRhinoGetObject go;
	go.SetCommandPrompt( L"Select master curve and curve to duplicate" );
	go.SetGeometryFilter( CRhinoGetObject::curve_object );

	CRhinoGet::result res = go.GetObjects(2,2);
	if( res == CRhinoGet::object )
	{
		const CRhinoObjRef& obj_ref = go.Object( 0 );
		masterCurve = obj_ref.Curve();
		if( !masterCurve ) {
			wStr.Format( L"The selected master object is not a valid curve");
			RhinoMessageBox( wStr, PlugIn()->PlugInName(), MB_OK );
			return CRhinoCommand::failure;
		}
		const CRhinoObjRef& curveToDuplicateObj_ref = go.Object( 1 );
		curveToDuplicate = curveToDuplicateObj_ref.Curve();
		// Get Object from ref for Transform method (???)
		objToDuplicate=curveToDuplicateObj_ref.Object();
		if( !curveToDuplicate ) {
			wStr.Format( L"The selected dupli object is not a valid curve");
			RhinoMessageBox( wStr, PlugIn()->PlugInName(), MB_OK );
			return CRhinoCommand::failure;
		}
	}
	wStr.Format( L"Il processo di curve è finito");
	RhinoMessageBox( wStr, PlugIn()->PlugInName(), MB_OK );

	/****************************** NUMBER OF DUPLICATION PROMPTS ************************************/
	CRhinoGetInteger gi;
	gi.SetCommandPrompt( L"Number of segments" );
	gi.SetDefaultInteger( 2 );
	gi.SetLowerLimit( 2 );
	gi.SetUpperLimit( 100 );
	gi.GetInteger();

	if( gi.Result() == CRhinoGet::cancel )
		return CRhinoCommand::cancel;

	if( gi.Result() != CRhinoGet::number )
		return CRhinoCommand::failure;

	int count = gi.Number();
	//count++;
	ON_SimpleArray<double> t( count );
	t.SetCount( count );

	int i;
	for( i = 0; i < count; i++ )
	{
		double param = (double)i / ((double)count-1);
		t[i] = param;
	}

	ON_Xform xform;
	//xform.Identity();

	ON_2dexMap group_map;

	if( masterCurve->GetNormalizedArcLengthPoints(count, (double*)&t[0], (double*)&t[0]) )
	{
		ON_3dPoint curveToDuplicateStartPoint=curveToDuplicate->PointAtStart();
		
		
		
		for( i = 0; i < count; i++ )
		{
			ON_3dPoint duplicationPoint = masterCurve->PointAt( t[i] );
			xform.Translation( duplicationPoint - curveToDuplicateStartPoint );
			//xform.Translation(2*i,2*i,2*i);
			CRhinoObject* duplicate = context.m_doc.TransformObject(objToDuplicate, xform, true, false, true );
			//CRhinoObject* duplicate = objToDuplicate->Duplicate();
			context.m_doc.AddPointObject( duplicationPoint );
			if( duplicate ) context.m_doc.Redraw(); //RhinoUpdateObjectGroups( duplicate, group_map );
			else {
				delete duplicate; return CRhinoCommand::failure;
			}
		}
		return CRhinoCommand::success; 
	}
}

#pragma endregion

//
// END CurveDuplicator command
//
////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
