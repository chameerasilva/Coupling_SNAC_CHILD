/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
**
** Copyright (C), 2003,
**	Steve Quenette, 110 Victoria Street, Melbourne, Victoria, 3053, Australia.
**	Californian Institute of Technology, 1200 East California Boulevard, Pasadena, California, 91125, USA.
**	University of Texas, 1 University Station, Austin, Texas, 78712, USA.
**
** Authors:
**	Stevan M. Quenette, Senior Software Engineer, VPAC. (steve@vpac.org)
**	Stevan M. Quenette, Visitor in Geophysics, Caltech.
**	Luc Lavier, Research Scientist, The University of Texas. (luc@utig.ug.utexas.edu)
**	Luc Lavier, Research Scientist, Caltech.
**
** This program is free software; you can redistribute it and/or modify it
** under the terms of the GNU General Public License as published by the
** Free Software Foundation; either version 2, or (at your option) any
** later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
**
** $Id: Output.c 3125 2005-07-25 21:32:45Z EunseoChoi $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include <StGermain/StGermain.h>
#include <StGermain/FD/FD.h>
#include "Snac/Snac.h"
#include "types.h"
#include "Output.h"
#include "Context.h"
#include "Element.h"
#include "Register.h"
#include <stdio.h>
#include <math.h>
#include <assert.h>


void _SnacViscoPlastic_WritePlasticStrain( void* _context ) {
	Snac_Context*				context = (Snac_Context*) _context;

	if( isTimeToDump( context ) )
		_SnacViscoPlastic_DumpPlasticStrain( context );

	if( isTimeToCheckpoint( context ) )
		_SnacViscoPlastic_CheckpointPlasticStrain( context );
}


void _SnacViscoPlastic_DumpPlasticStrain( void* _context ) {
	Snac_Context*				context = (Snac_Context*) _context;
	SnacViscoPlastic_Context*	contextExt = ExtensionManager_Get(
												context->extensionMgr,
												context,
												SnacViscoPlastic_ContextHandle );
	Element_LocalIndex			element_lI;

#if DEBUG
	printf( "In %s()\n", __func__ );
#endif
	
	for( element_lI = 0; element_lI < context->mesh->elementLocalCount; element_lI++ ) {
		Snac_Element* 				element = Snac_Element_At( context, element_lI );
		SnacViscoPlastic_Element*	elementExt = ExtensionManager_Get(
													context->mesh->elementExtensionMgr,
													element,
													SnacViscoPlastic_ElementHandle );
		float plasticStrain = elementExt->aps;
		fwrite( &plasticStrain, sizeof(float), 1, contextExt->plStrainOut );
	}
	fflush( contextExt->plStrainOut );
}


void _SnacViscoPlastic_CheckpointPlasticStrain( void* _context ) {
	Snac_Context*				context = (Snac_Context*) _context;
	SnacViscoPlastic_Context*	contextExt = ExtensionManager_Get(
												context->extensionMgr,
												context,
												SnacViscoPlastic_ContextHandle );
	Element_LocalIndex			element_lI;

#if DEBUG
	printf( "In %s()\n", __func__ );
#endif
	
	for( element_lI = 0; element_lI < context->mesh->elementLocalCount; element_lI++ ) {
		Snac_Element* 				element = Snac_Element_At( context, element_lI );
		SnacViscoPlastic_Element*	elementExt = ExtensionManager_Get(
													context->mesh->elementExtensionMgr,
													element,
													SnacViscoPlastic_ElementHandle );
		float avgPlasticStrain = elementExt->aps;
		Tetrahedra_Index	tetra_I;

		for( tetra_I = 0; tetra_I < Tetrahedra_Count; tetra_I++ ) {
			float plasticStrain = elementExt->plasticStrain[tetra_I];
			fwrite( &plasticStrain, sizeof(float), 1, contextExt->plStrainCheckpoint );
		}
		fflush( contextExt->plStrainCheckpoint );


		fwrite( &avgPlasticStrain, sizeof(float), 1, contextExt->avgPlStrainCheckpoint );
	}
	fflush( contextExt->avgPlStrainCheckpoint );
}


void _SnacViscoPlastic_WriteViscosity( void* _context ) {
	Snac_Context*				context = (Snac_Context*) _context;

	if( isTimeToDump( context ) )
		_SnacViscoPlastic_DumpViscosity( context );

	if( isTimeToCheckpoint( context ) )
		_SnacViscoPlastic_CheckpointViscosity( context );
}


void _SnacViscoPlastic_DumpViscosity( void* _context ) {
	Snac_Context*				context = (Snac_Context*) _context;
	SnacViscoPlastic_Context*	contextExt = ExtensionManager_Get(
												context->extensionMgr,
												context,
												SnacViscoPlastic_ContextHandle );
	Element_LocalIndex			element_lI;

#if DEBUG
	printf( "In %s()\n", __func__ );
#endif
	
	for( element_lI = 0; element_lI < context->mesh->elementLocalCount; element_lI++ ) {
		Snac_Element* 				element = Snac_Element_At( context, element_lI );
		SnacViscoPlastic_Element*	elementExt = ExtensionManager_Get(
													context->mesh->elementExtensionMgr,
													element,
													SnacViscoPlastic_ElementHandle );
		Tetrahedra_Index			tetra_I;
		double					    viscosity = 0.0f;
		float						logviscosity = 0.0f;

		/* Take average of tetra viscosity for the element */
		for( tetra_I = 0; tetra_I < Tetrahedra_Count; tetra_I++ )
			viscosity += elementExt->viscosity[tetra_I]/Tetrahedra_Count;
		assert(viscosity>0.0);
		logviscosity = log10(viscosity);
		fwrite( &logviscosity, sizeof(float), 1, contextExt->viscOut );
	}
	fflush( contextExt->viscOut );
}


void _SnacViscoPlastic_CheckpointViscosity( void* _context ) {
	Snac_Context*				context = (Snac_Context*) _context;
	SnacViscoPlastic_Context*	contextExt = ExtensionManager_Get(
												context->extensionMgr,
												context,
												SnacViscoPlastic_ContextHandle );
	Element_LocalIndex			element_lI;
	
#if DEBUG
	printf( "In %s()\n", __func__ );
#endif
	
	for( element_lI = 0; element_lI < context->mesh->elementLocalCount; element_lI++ ) {
		Snac_Element* 					element = Snac_Element_At( context, element_lI );
		SnacViscoPlastic_Element*		elementExt = ExtensionManager_Get(
														context->mesh->elementExtensionMgr,
														element,
														SnacViscoPlastic_ElementHandle );
		Tetrahedra_Index		tetra_I;
		double					viscosity = 0.0f;
		float					logviscosity = 0.0f;

		for( tetra_I = 0; tetra_I < Tetrahedra_Count; tetra_I++ ) {
			viscosity = elementExt->viscosity[tetra_I];
			assert(viscosity>0.0);
			logviscosity = log10(viscosity);
			fwrite( &logviscosity, sizeof(float), 1, contextExt->viscCheckpoint );
		}
	}
	fflush( contextExt->viscCheckpoint );
}


#if 0
void _SnacViscoPlastic_DumpPlasticStrainTensor( void* _context ) {
	Snac_Context*				context = (Snac_Context*) _context;
	SnacViscoPlastic_Context*			contextExt = ExtensionManager_Get(
							context->extensionMgr,
							context,
							SnacViscoPlastic_ContextHandle );

#if DEBUG
	printf( "In %s()\n", __func__ );
#endif


	if( context->timeStep ==0 || (context->timeStep-1) % context->dumpEvery == 0 ) {
		Element_LocalIndex			element_lI;

		for( element_lI = 0; element_lI < context->mesh->elementLocalCount; element_lI++ ) {
			Snac_Element* 				element = Snac_Element_At( context, element_lI );
			SnacViscoPlastic_Element*			elementExt = ExtensionManager_Get(
																				  context->mesh->elementExtensionMgr,
																				  element,
																				  SnacViscoPlastic_ElementHandle );
			const Snac_Material* material = &context->materialProperty[element->material_I];
			Tetrahedra_Index tetra_I;


			if( material->yieldcriterion == druckerprager ) {
				for( tetra_I = 0; tetra_I < Tetrahedra_Count; tetra_I++ ) {
					Index i,j;
					float tensor[3][3];
					for(i=0;i<3;i++)
						for(j=0;j<3;j++) {
							tensor[i][j] = elementExt->plasticstrainTensor[tetra_I][i][j];
							fwrite( &tensor[i][j], sizeof(float), 1, contextExt->plstrainTensorOut );
						}
				}
			}
			else if( material->yieldcriterion == mohrcoulomb ) {
				for( tetra_I = 0; tetra_I < Tetrahedra_Count; tetra_I++ ) {
					float tetraStrain;
					tetraStrain = elementExt->plasticStrain[tetra_I];
					fwrite( &tetraStrain, sizeof(float), 1, contextExt->plstrainTensorOut );
				}
			}
		}
		fflush( contextExt->plstrainTensorOut );
	}
}
#endif
