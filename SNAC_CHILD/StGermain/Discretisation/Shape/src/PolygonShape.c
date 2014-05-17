/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
**
** Copyright (C), 2003, Victorian Partnership for Advanced Computing (VPAC) Ltd, 110 Victoria Street, Melbourne, 3053, Australia.
**
** Authors:
**	Stevan M. Quenette, Senior Software Engineer, VPAC. (steve@vpac.org)
**	Patrick D. Sunter, Software Engineer, VPAC. (pds@vpac.org)
**	Luke J. Hodkinson, Computational Engineer, VPAC. (lhodkins@vpac.org)
**	Siew-Ching Tan, Software Engineer, VPAC. (siew@vpac.org) ) 
**	Alan H. Lo, Computational Engineer, VPAC. (alan@vpac.org)
**	Raquibul Hassan, Computational Engineer, VPAC. (raq@vpac.org)
**
**  This library is free software; you can redistribute it and/or
**  modify it under the terms of the GNU Lesser General Public
**  License as published by the Free Software Foundation; either
**  version 2.1 of the License, or (at your option) any later version.
**
**  This library is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
**  Lesser General Public License for more details.
**
**  You should have received a copy of the GNU Lesser General Public
**  License along with this library; if not, write to the Free Software
**  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
**
** $Id: PolygonShape.c 3869 2006-10-16 13:42:59Z SteveQuenette $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include <Base/Base.h>
#include <Discretisation/Geometry/Geometry.h>

#include "types.h"
#include "ShapeClass.h"
#include "PolygonShape.h"

#include <assert.h>
#include <string.h>
#include <math.h>

/* Textual name of this class */
const Type PolygonShape_Type = "PolygonShape";

/*----------------------------------------------------------------------------------------------------------------------------------
** Constructors
*/

PolygonShape* PolygonShape_New(
		Name                                  name,
		Dimension_Index                       dim,
		XYZ                                   centre, 
		double                                alpha,
		double                                beta,
		double                                gamma,
		Coord_List                            vertexList,
		Index                                 vertexCount,
		double                                startZ,
		double                                endZ )
{
	PolygonShape* self = (PolygonShape*)_PolygonShape_DefaultNew( name );

	PolygonShape_InitAll( 
		self, 
		dim,
		centre,
		alpha,
		beta,
		gamma,
		vertexList,
		vertexCount,
		startZ,
		endZ);
	return self;
}

PolygonShape* _PolygonShape_New(
		SizeT                                 _sizeOfSelf, 
		Type                                  type,
		Stg_Class_DeleteFunction*             _delete,
		Stg_Class_PrintFunction*              _print,
		Stg_Class_CopyFunction*               _copy, 
		Stg_Component_DefaultConstructorFunction* _defaultConstructor,
		Stg_Component_ConstructFunction*      _construct,
		Stg_Component_BuildFunction*          _build,
		Stg_Component_InitialiseFunction*     _initialise,
		Stg_Component_ExecuteFunction*        _execute,
		Stg_Component_DestroyFunction*        _destroy,		
		Stg_Shape_IsCoordInsideFunction*      _isCoordInside,
		Stg_Shape_CalculateVolumeFunction*    _calculateVolume,
		Name                                  name )
{
	PolygonShape* self;
	
	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof(PolygonShape) );
	self = (PolygonShape*)_Stg_Shape_New( 
			_sizeOfSelf,
			type,
			_delete,
			_print,
			_copy,
			_defaultConstructor,
			_construct,
			_build,
			_initialise,
			_execute,
			_destroy,		
			_isCoordInside ,
			_calculateVolume,
			name );
	
	/* General info */

	/* Virtual Info */
	self->_isCoordInside = _isCoordInside;
	
	return self;
}

void _PolygonShape_Init( void* polygon, Coord_List vertexList, Index vertexCount, double startZ, double endZ ) {
	PolygonShape* self = (PolygonShape*)polygon;
	
	self->vertexList = Memory_Alloc_Array( Coord, vertexCount, "vertexList" );
	memcpy( self->vertexList , vertexList, sizeof(Coord) * vertexCount );
	self->vertexCount = vertexCount;
	self->startZ = startZ;
	self->endZ   = endZ;
}


void PolygonShape_InitAll( 
		void*                                 polygon, 
		Dimension_Index                       dim, 
		Coord                                 centre,
		double                                alpha,
		double                                beta,
		double                                gamma,
		Coord_List                            vertexList,
		Index                                 vertexCount,
		double                                startZ,
		double                                endZ	)
{
	PolygonShape* self = (PolygonShape*)polygon;
	Stg_Shape_InitAll( self, dim, centre, alpha, beta, gamma);
	_PolygonShape_Init( self, vertexList, vertexCount, startZ, endZ );
}
	

/*------------------------------------------------------------------------------------------------------------------------
** Virtual functions
*/

void _PolygonShape_Delete( void* polygon ) {
	PolygonShape*       self = (PolygonShape*)polygon;
	Coord_List     vertexList = self->vertexList;

	Memory_Free( vertexList );

	
	/* Delete parent */
	_Stg_Shape_Delete( self );
}


void _PolygonShape_Print( void* polygon, Stream* stream ) {
	PolygonShape* self = (PolygonShape*)polygon;
	
	/* Print parent */
	_Stg_Shape_Print( self, stream );
}



void* _PolygonShape_Copy( void* polygon, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap ) {
	PolygonShape*	self = (PolygonShape*)polygon;
	PolygonShape*	newPolygonShape;
	
	newPolygonShape = (PolygonShape*)_Stg_Shape_Copy( self, dest, deep, nameExt, ptrMap );

	newPolygonShape->vertexList = Memory_Alloc_Array( Coord, self->vertexCount, "vertexList" );
	memcpy( newPolygonShape->vertexList , self->vertexList, sizeof(Coord) * self->vertexCount );

	newPolygonShape->vertexList  = self->vertexList;
	newPolygonShape->vertexCount = self->vertexCount;
	newPolygonShape->startZ      = self->startZ;
	newPolygonShape->endZ        = self->endZ;
	
	return (void*)newPolygonShape;
}

void* _PolygonShape_DefaultNew( Name name ) {
	return (void*) _PolygonShape_New(
			sizeof(PolygonShape),
			PolygonShape_Type,
			_PolygonShape_Delete,
			_PolygonShape_Print,
			_PolygonShape_Copy,
			_PolygonShape_DefaultNew,
			_PolygonShape_Construct,
			_PolygonShape_Build,
			_PolygonShape_Initialise,
			_PolygonShape_Execute,
			_PolygonShape_Destroy,
			_PolygonShape_IsCoordInside,
			_PolygonShape_CalculateVolume,
			name );
}


void _PolygonShape_Construct( void* polygon, Stg_ComponentFactory* cf, void* data ) {
	PolygonShape*           self       = (PolygonShape*)polygon;
	Index                   vertexCount;
	Index                   vertex_I;
	Coord_List              vertexList;
	double                  startZ;
	double                  endZ;
	double*                 coord;
	Dictionary_Entry_Value* optionSet;
	Dictionary_Entry_Value* optionsList;
	Dictionary*             dictionary  = Dictionary_GetDictionary( cf->componentDict, self->name );
	Stream*                 stream      = cf->infoStream;
	Stream*                 errorStream = Journal_Register( Error_Type, self->type );
	
	_Stg_Shape_Construct( self, cf, data );

	startZ = Stg_ComponentFactory_GetDouble( cf, self->name, "startZ", 0.0 );
	endZ   = Stg_ComponentFactory_GetDouble( cf, self->name, "endZ",   0.0 );

	optionsList = Dictionary_Get( dictionary, "verticies" );
	
	vertexCount = Dictionary_Entry_Value_GetCount(optionsList);
	Journal_Firewall( vertexCount >= 3, errorStream, 
			"To few verticies given in trying to build shape '%s' named '%s'.\n"
			"A polygon needs at least three verticies.\n",
			self->type, self->name );

	/* Allocate space */
	vertexList = Memory_Alloc_Array( Coord , vertexCount, "Vertex Array" );
	memset( vertexList, 0, vertexCount * sizeof(Coord) );

	Stream_Indent( stream );
	for ( vertex_I = 0 ; vertex_I < vertexCount ; vertex_I++) { 
		optionSet = Dictionary_Entry_Value_GetElement(optionsList, vertex_I );
		coord = vertexList[vertex_I];
		/* Read Vertex */
		coord[ I_AXIS ] = Dictionary_Entry_Value_AsDouble( Dictionary_Entry_Value_GetMember( optionSet, "x"));
		coord[ J_AXIS ] = Dictionary_Entry_Value_AsDouble( Dictionary_Entry_Value_GetMember( optionSet, "y"));

		/* Print Position */
		Journal_PrintfL( stream, 2, "(%0.3g, %0.3g)\n", coord[ I_AXIS ], coord[ J_AXIS ] );
	}
	Stream_UnIndent( stream );

	_PolygonShape_Init( self, vertexList, vertexCount, startZ, endZ );

	Memory_Free( vertexList );
}

void _PolygonShape_Build( void* polygon, void* data ) {
	PolygonShape*	self = (PolygonShape*)polygon;

	_Stg_Shape_Build( self, data );
}
void _PolygonShape_Initialise( void* polygon, void* data ) {
	PolygonShape*	self = (PolygonShape*)polygon;
	
	_Stg_Shape_Initialise( self, data );
}
void _PolygonShape_Execute( void* polygon, void* data ) {
	PolygonShape*	self = (PolygonShape*)polygon;
	
	_Stg_Shape_Execute( self, data );
}
void _PolygonShape_Destroy( void* polygon, void* data ) {
	PolygonShape*	self = (PolygonShape*)polygon;
	
	_Stg_Shape_Destroy( self, data );
}

/*--------------------------------------------------------------------------------------------------------------------------
** Public Functions
*/
	
/*---------------------------------------------------------------------------------------------------------------------
** Private Member functions
*/
/* Algorithm describe Paul Bourke's page http://astronomy.swin.edu.au/~pbourke/geometry/insidepoly/ (see solution 2)
 *
 * Algorithm works by summing the angles between the test coordinate and each pair of verticies that make up an edge 
 * in the polygon. An inside point will give an angle of 2pi and and outside point will give an angle of 0 */

Bool _PolygonShape_IsCoordInside( void* polygon, Coord testCoord ) {
	PolygonShape*        self                = (PolygonShape*) polygon;
	Index           vertexCount         = self->vertexCount;
	Coord_List      vertexList          = self->vertexList;
	XYZ             vectorToStartVertex = { 0.0, 0.0, 0.0 };
	XYZ             vectorToEndVertex   = { 0.0, 0.0, 0.0 };
	XYZ             crossproduct        = { 0.0, 0.0, 0.0 };
	double          currAngle;
	double          totalAngle          = 0.0;
	Index           vertex_I;
	double*         startVertex;
	double*         endVertex;

	/* Check to make sure that the coordinate is within startZ and endZ in 3D */
	if ( self->dim == 3 && ( testCoord[ K_AXIS ] < self->startZ || testCoord[ K_AXIS ] > self->endZ ))
		return False;	

	for ( vertex_I = 0 ; vertex_I < vertexCount ; vertex_I++ ) {
		/* Get verticies of current edge */
		startVertex = vertexList[ vertex_I ];
		endVertex   = vertexList[ (vertex_I + 1) % vertexCount ];

		/* Work out vectors */
		StGermain_VectorSubtraction( vectorToStartVertex, testCoord, startVertex, 2 );
		StGermain_VectorSubtraction( vectorToEndVertex,   testCoord, endVertex,   2 );

		/* Work out angle - just by doing dot product - will always be positive */
		currAngle = StGermain_AngleBetweenVectors( vectorToStartVertex, vectorToEndVertex, 2 );

		/* Work out 'sign' of angle but working out cross product */
		StGermain_VectorCrossProduct( crossproduct, vectorToEndVertex, vectorToStartVertex );

		if ( crossproduct[ K_AXIS ] > 0.0 )
			totalAngle += currAngle;
		else
			totalAngle -= currAngle;
	}


	/* work out whether the coord is within the polygon */
	if ( fabs( totalAngle ) < M_PI )
		return False;
	else  
		return True;
}


double _PolygonShape_CalculateVolume( void* polygon ) {
	assert( 0 );
	return 0.0;
}
