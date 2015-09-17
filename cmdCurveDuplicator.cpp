/////////////////////////////////////////////////////////////////////////////
// cmdCurveDuplicator.cpp : command file
//

#include "StdAfx.h"
#include "CurveDuplicatorPlugIn.h"
#include "CurveTypeUtils.h"

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

	ON_wString wStr;
	bool perpendicular = false;
	ON_Curve* masterCurve; // *** Curve in which lie the duplicated curve
	const ON_Curve* curveToDuplicate; 
	ON_3dPoint curveToDuplicateSelectionClosestPoint;

	/*********************************************************************************
	BEGIN USER INTERACTION
	*********************************************************************************/
	/* 
	GET CURVE AND PROMPTS 
	*/
	CRhinoGetObject go;
	go.SetCommandPrompt( L"Select master curve and curve to duplicate" );
	go.SetGeometryFilter( CRhinoGetObject::curve_object );

	const ON_Curve* tempCurve;

	CRhinoGet::result res = go.GetObjects(2,2);
	if( res == CRhinoGet::object )
	{
		const CRhinoObjRef& obj_ref = go.Object( 0 ); // *** Get the reference to master curve
		tempCurve = obj_ref.Curve(); // Get the pointer to master curve (temporary)
		if( tempCurve ) {
			masterCurve = const_cast<ON_Curve*>(tempCurve); // Master curve - This for reverse curve method that doesn't accept const object
		}
		else {
			wStr.Format( L"The selected master object is not a valid curve");
			RhinoMessageBox( wStr, PlugIn()->PlugInName(), MB_OK );
			return CRhinoCommand::failure;
		}
		const CRhinoObjRef& curveToDuplicateObj_ref = go.Object( 1 ); // Get 
		curveToDuplicate = curveToDuplicateObj_ref.Curve();
		if( !curveToDuplicate) {
			wStr.Format( L"The selected dupli object is not a valid curve");
			RhinoMessageBox( wStr, PlugIn()->PlugInName(), MB_OK );
			return CRhinoCommand::failure;
		}
		else {
			const ON_Interval subDomain = ON_Interval();
			curveToDuplicateSelectionClosestPoint = go.Object(1).m_point;
			curveToDuplicate->GetClosestPoint(curveToDuplicateSelectionClosestPoint,0,0.00000001,&subDomain);
			//wStr.Format( L"The point is x %f y %f ",curveToDuplicateSelectionClosestPoint.x, curveToDuplicateSelectionClosestPoint.y);
			//RhinoMessageBox( wStr, PlugIn()->PlugInName(), MB_OK );
		}
	}
	if( go.CommandResult() != success ) return go.CommandResult();

	/*
	NUMBER OF DUPLICATION PROMPTS 
	*/
	CRhinoGetInteger gi;
	gi.SetCommandPrompt( L"Number of duplication" );
	gi.SetDefaultInteger( 2 );
	gi.SetLowerLimit( 2 );
	gi.SetUpperLimit( 100 );
	gi.GetInteger();

	if( gi.Result() == CRhinoGet::cancel )
		return CRhinoCommand::cancel;

	if( gi.Result() != CRhinoGet::number )
		return CRhinoCommand::failure;

	int duplicationCount = gi.Number();


	/*********************************************************************************
	END USER INTERACTION - BEGIN DATA PROCESS
	*********************************************************************************/

	/*
	For first get the direction of master curve for placement duplicated curve.
	NEED MORE SPECIFICATIONS ON HOW TO POSITION CURVES
	For each duplication calculates the duplication point on the master curve, 
	get the angle on point and apply to duplicated curve with translation
	*/

	ON_SimpleArray<double> masterCurveParameters ( duplicationCount );
	masterCurveParameters.SetCount( duplicationCount );
	int i;

	/*
	Get curve parameters for analize direction (è inutile ? c'è un'altro metodo) ( Double loops to avoid calling GetNormalizedArcLengthPoints for every duplications)
	*/
	for( i = 0; i < duplicationCount; i++ ) //
	{
		double parameter = (double)i / ((double)duplicationCount-1);
		masterCurveParameters[i] = parameter;
	}
	masterCurve->GetNormalizedArcLengthPoints(duplicationCount, (double*)&masterCurveParameters[0], (double*)&masterCurveParameters[0]);
	
	/*ON_3dPoint originalCurveStartPoint = masterCurve->PointAt( masterCurveParameters[0] ); 
	context.m_doc.AddPointObject( originalCurveStartPoint );*/
	
	/* 
	Get the master curve angle (direction) at start respect to x axis
	*/
	double masterCurveAngleDegreesOnStart =curveAngleDeegresAtPointRespectRefAxis(masterCurve,RhinoActiveCPlane().xaxis,masterCurveParameters[0]);

	if(masterCurveAngleDegreesOnStart<0) // Curve direction is antiorario
	{
		wStr.Format( L"direction on start is counterclockwise: reverse the curve");
		RhinoMessageBox( wStr, PlugIn()->PlugInName(), MB_OK );
		masterCurve->Reverse();	// *** Reverse the curve
		
		/*
		Reset curve parameters
		*/
		for( i = 0; i < duplicationCount; i++ ) //
		{
			double parameter = (double)i / ((double)duplicationCount-1);
			masterCurveParameters[i] = parameter;
		}
		masterCurve->GetNormalizedArcLengthPoints(duplicationCount, (double*)&masterCurveParameters[0], (double*)&masterCurveParameters[0]);
		masterCurveAngleDegreesOnStart =curveAngleDeegresAtPointRespectRefAxis(masterCurve,RhinoActiveCPlane().xaxis,masterCurveParameters[0]);
	}

	ON_Curve* duplicatedCurve; // *** Curve to duplicate

	for( i = 0; i < duplicationCount; i++ )
	{
		ON_3dPoint duplicationPoint = masterCurve->PointAt( masterCurveParameters[i] );
		
		double  duplicatedCurveAngleDegrees;
		double  masterCurveAngleDegrees;
		duplicatedCurve = curveToDuplicate->DuplicateCurve();
		
		if( masterCurve->GetClosestPoint(duplicationPoint, &masterCurveParameters[i])) 	// *** Get the closest point to parameter in master curve		
		{
			/* 
			Calculate the master curve point angle respect to x axis
			*/
			masterCurveAngleDegrees = curveAngleDeegresAtPointRespectRefAxis(masterCurve,RhinoActiveCPlane().xaxis,masterCurveParameters[i]);			
			/*
			Get the duplicated curve vector only for get the duplicatedCurve angle
			*/
			ON_3dVector duplicatedCurveVector;
			duplicatedCurveVector = ON_3dVector(duplicatedCurve->PointAtEnd()-duplicatedCurve->PointAtStart());
			duplicatedCurveVector.Unitize();
			/* 
			Calculate the duplicated curve (curve to duplicate) angle respect to x axis to reset the rotation 
			*/
			duplicatedCurveAngleDegrees = vectorAngleDeegresRespectRefAxis(duplicatedCurveVector,RhinoActiveCPlane().xaxis);

		}
		else {
			return CRhinoCommand::failure;
		}
		/*
		Resetting the rotation and position
		*/
		if(false) {
			duplicatedCurve->Rotate(((duplicatedCurveAngleDegrees) * ON_PI / 180.0),RhinoActiveCPlane().zaxis,curveToDuplicateSelectionClosestPoint); // *** Reset rotation
			duplicatedCurve->Translate(ON_3dPoint(-curveToDuplicateSelectionClosestPoint.x,-curveToDuplicateSelectionClosestPoint.y,0));
		}
		/*
		Positioning and rotate duplicatedCurve on masterCurve duplication point
		*/
		if(true) {
			duplicatedCurve->Translate(duplicationPoint-curveToDuplicateSelectionClosestPoint); // Positioning curve on masterCurve duplication point
			duplicatedCurve->Rotate((-(masterCurveAngleDegrees-masterCurveAngleDegreesOnStart) * ON_PI / 180.0),RhinoActiveCPlane().zaxis,duplicationPoint ); // *** Apply rotation
		}
		
		context.m_doc.AddCurveObject(*duplicatedCurve /*,&attribs*/ );			
	}
	context.m_doc.Redraw();

	return CRhinoCommand::success; 
}

#pragma endregion

//
// END CurveDuplicator command
//
////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
