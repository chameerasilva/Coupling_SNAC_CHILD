/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 **
 ** Copyright (C), 2003,
 **	Steve Quenette, 110 Victoria Street, Melbourne, Victoria, 3053, Australia.
 **	Californian Institute of Technology, 1200 East California Boulevard, Pasadena, California, 91125, USA.
 **	University of Texas, 1 University Station, Austin, Texas, 78712, USA.
 **
 ** Authors:
**           Colin Stark, Doherty Research Scientist, Lamont-Doherty Earth Observatory (cstark@ldeo.columbia.edu)
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
 ** $Id: CreateWeakPoints.c $
 **
 **~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include <StGermain/StGermain.h>
#include <StGermain/FD/FD.h>
#include "Snac/Snac.h"
#include "types.h"
#include "Context.h"
#include "Register.h"
#include "CreateWeakPoints.h"
#include "Snac/Plastic/Plastic.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#ifndef PI
	#ifndef M_PIl
		#ifndef M_PI
			#define PI 3.14159265358979323846
		#else
			#define PI M_PI
		#endif
	#else
		#define PI M_PIl
	#endif
#endif
#define min(a,b) ((a)<(b) ? (a):(b))
#ifndef PATH_MAX
	#define PATH_MAX 1024
#endif

#define DEBUG
//#define DEBUG2


/*
 *----------------------------------------------------------------------
 *
 * SnacHillSlope_CreateWeakPoints --
 *
 *      tbd
 *
 * Returns:
 *	Nothing directly
 *
 * Side effects:
 *      Altered mesh with weak points induced by forced plastic deformation
 *
 *----------------------------------------------------------------------
 */
void SnacHillSlope_CreateWeakPoints( void* _context ) {
    Snac_Context		*context = (Snac_Context*)_context;
    SnacHillSlope_Context	*contextExt = ExtensionManager_Get(context->extensionMgr,
								   context,
								   SnacHillSlope_ContextHandle );
    Mesh			*mesh = context->mesh;
    MeshLayout			*layout = (MeshLayout*)mesh->layout;
    HexaMD			*decomp = (HexaMD*)layout->decomp;

    double             	        fractionWeakPoints = contextExt->fractionWeakPoints;

    const int			full_I_element_range=decomp->elementGlobal3DCounts[0];
    const int			full_J_element_range=decomp->elementGlobal3DCounts[1];
    const int			full_K_element_range=decomp->elementGlobal3DCounts[2];

    Element_LocalIndex		element_lI;
    Element_GlobalIndex		element_gI;
    int                         index, index_I,index_J,index_K;
    IJK				ijk;


    /*
     *  Bail now if initial elastic equilibrium has not been reached on all threads
     *
     *  Also bail if we're solving elastic eqm only
     */
    if(!contextExt->consensusElasticStabilizedFlag || contextExt->seedingCompletedFlag
       || (contextExt->solveElasticEqmOnlyFlag)) {
#ifdef DEBUG2
	fprintf(stderr,"Bailing from CWP: consensusElasticStabilized=%d, seedingCompletedFlag=%d, solveElasticEqmOnlyFlag=%d\n",
		contextExt->consensusElasticStabilizedFlag, contextExt->seedingCompletedFlag, contextExt->solveElasticEqmOnlyFlag);
#endif
	return;
    }

    //    fprintf(stderr, "CWP\n");

    /*
     *  Insert weak (low cohesion) points at N random elements by forcing plastic strain
     */
    if(fractionWeakPoints>=0.0 && fractionWeakPoints<=1.0) {
	unsigned int		rngSeed = contextExt->rngSeed;
	double			x_subDomainFraction = contextExt->xSubDomainFraction;
	double			y_subDomainFraction = contextExt->ySubDomainFraction;
	double			z_subDomainFraction = contextExt->zSubDomainFraction;
	double			x_subDomainLeftPosition = contextExt->xSubDomainLeft;
	double			subDomainMinCohesion = contextExt->subDomainMinCohesion;
	double			subDomainMaxCohesion = contextExt->subDomainMaxCohesion;
	unsigned int		numberSubDomainPoints,numberWeakPoints;
	unsigned int		*shuffleIndexListPtr, *randomNumberListPtr;
	int			subDomain_I_element_range,subDomain_J_element_range,subDomain_K_element_range,subDomain_I_element_left_position;
	/*  Report HillSlope plugin variables picked up (?) from xml parameter file */
	Journal_Printf( context->snacInfo, "\tRNG seed = %u\n", rngSeed );
	Journal_Printf( context->snacInfo, "\tFraction of weak points = %g\n", fractionWeakPoints );
	Journal_Printf( context->snacInfo, "\tx subDomain fraction = %g\n", x_subDomainFraction );
	Journal_Printf( context->snacInfo, "\ty subDomain fraction = %g\n", y_subDomainFraction );
	Journal_Printf( context->snacInfo, "\tz subDomain fraction = %g\n", z_subDomainFraction );
	Journal_Printf( context->snacInfo, "\tx subDomain left position = %g\n", x_subDomainLeftPosition );

	/*
	 *  Define portion of mesh (cells) that need weak point "seeding"
	 *    - lots of sloppy float/int casting back and forth here
	 */
	subDomain_I_element_range = (int)(full_I_element_range*x_subDomainFraction);
	subDomain_J_element_range = (int)(full_J_element_range*y_subDomainFraction);
	subDomain_K_element_range = (int)(full_K_element_range*z_subDomainFraction);
	if ( x_subDomainLeftPosition < 0.0 || subDomain_I_element_range+(int)(full_I_element_range*x_subDomainLeftPosition+0.5) > full_I_element_range)
	    subDomain_I_element_left_position = (full_I_element_range-subDomain_I_element_range)/2;
	else
	    subDomain_I_element_left_position = (int)(full_I_element_range*x_subDomainLeftPosition+0.5);

	Journal_Printf( context->snacInfo, "\tx subDomain element range = %d/%d\n", subDomain_I_element_range,full_I_element_range );
	Journal_Printf( context->snacInfo, "\ty subDomain element range = %d/%d\n", subDomain_J_element_range,full_J_element_range );
	Journal_Printf( context->snacInfo, "\tz subDomain element range = %d/%d\n", subDomain_K_element_range,full_K_element_range );
	Journal_Printf( context->snacInfo, "\tx subDomain element left position = %d/%d\n\n", subDomain_I_element_left_position,full_I_element_range );

	numberSubDomainPoints = subDomain_I_element_range*subDomain_J_element_range*subDomain_K_element_range;
	numberWeakPoints = (unsigned int)((float)numberSubDomainPoints*fractionWeakPoints);
	Journal_Printf(context->snacInfo,  "Number of weak points = %u/%u\n", numberWeakPoints,numberSubDomainPoints);
	
	/*
	 *  Seed RNG (random number generator) using seed value from input xml file
	 */
	srand( rngSeed );
	shuffleIndexListPtr=(unsigned int *)malloc((size_t)(numberSubDomainPoints*sizeof(unsigned int)));
	randomNumberListPtr=(unsigned int *)malloc((size_t)(numberSubDomainPoints*sizeof(unsigned int)));
	/*
	 *  Create (1) ordered sequence of element indices, (2) parallel list of random variates
	 */
	index=0;
	for(index_K = 0; index_K < subDomain_K_element_range; index_K++) {
	    for(index_J = 0; index_J < subDomain_J_element_range; index_J++) {
		for(index_I = 0; index_I < subDomain_I_element_range; index_I++) {
		    /*
		     *  Work out the element index from the i,j,k and the required location of the subDomain
		     */
		    element_gI = index_I + subDomain_I_element_left_position
			+ full_I_element_range*(full_J_element_range-1-index_J)
			+ full_I_element_range*full_J_element_range*(index_K+(full_K_element_range-subDomain_K_element_range)/2);
		    element_lI = Mesh_ElementMapGlobalToLocal( mesh, element_gI );


		    /*
		     *  Impose heterogeneous cohesion within whole subDomain - if min/max values are non-neg
		     */
		    if(element_lI < mesh->elementLocalCount && subDomainMinCohesion>0.0) {
			Snac_Element		*element;
			Snac_Material	    	*material;
			SnacPlastic_Element	*plasticElement;
			Tetrahedra_Index	tetra_I;
			double			localRandomCohesion
			    = subDomainMinCohesion+((drand48()+drand48())/2.0)*(subDomainMaxCohesion-subDomainMinCohesion);
			/*
			 *  Naughty use of drand48 to generate triangular pdf variates spanning [subMin, subMax) 
			 */

			element = Snac_Element_At( context, element_lI );
			material = &context->materialProperty[element->material_I];
			plasticElement = ExtensionManager_Get(  mesh->elementExtensionMgr, element, SnacPlastic_ElementHandle );
			for( tetra_I = 0; tetra_I < Tetrahedra_Count; tetra_I++ ) {
			    plasticElement->plasticStrain[tetra_I] = PlasticStrainFromCohesion(material,localRandomCohesion);
#ifdef DEBUG2	    
			    if(tetra_I==0) 
				Journal_Printf( context->snacInfo,
						"Heterogeneous cohesion:  timeStep=%d  element (%d,%d,%d)#=%d->%d  cohesion=%g  plasticStrain=%g\n",
						context->timeStep, index_I,index_J,index_K, element_gI, element_lI, 
						localRandomCohesion, plasticElement->plasticStrain[tetra_I] );
#endif
			    
			}
		    } // End if

		    
		    shuffleIndexListPtr[index]=element_lI;
		    randomNumberListPtr[index]=rand();
		    index++;
		    if(index-1>numberSubDomainPoints) break;
		}
		if(index>0 && index-1>numberSubDomainPoints) break;
	    }
	    if(index>0 && index-1>numberSubDomainPoints) {fprintf(stderr, "We miscalculated the subDomain index\n"); break;}
	}
	/*
	 *  Sort the list of sub-domain local (ie global set pared down for this CPU) 
	 *  indices according to the random numbers
	 *    - i.e., randomize the index list
	 */
	if (numberWeakPoints>0) {
	    ShellSort( randomNumberListPtr, shuffleIndexListPtr, numberSubDomainPoints );
#ifdef DEBUG2
	    for(index = 0; index < numberSubDomainPoints; index++) {
		Journal_Printf( context->snacInfo, "\t SubDomain point #%d = %d, RNG var=%d\n", 
				index, shuffleIndexListPtr[index],randomNumberListPtr[index] );
	    }
#endif
	}
	/*
	 *  Now the subDomain index list is shuffled and the random number list is ordered
	 */


	/*
	 *  Loop over the shuffled subDomain index list BUT only up to the number of weak points within it
	 *   - i.e., subset the subDomain and allocate weak points to only a random fraction of them
	 */
	for(index = 0; index < numberWeakPoints; index++) {
	    element_lI = shuffleIndexListPtr[index];

#ifdef DEBUG2
	    Journal_Printf(context->snacInfo,  "Shuffled weak point # %d  -> %d \n", index,  element_lI);
#endif
	    
	    /*
	     *  At each point, force low cohesion by imposing a degree of plastic strain
	     */
	    if(element_lI < mesh->elementLocalCount) {
		Snac_Element		*element;
		Snac_Material	    	*material;
		SnacPlastic_Element	*plasticElement;
		Tetrahedra_Index	tetra_I;
		
		element = Snac_Element_At( context, element_lI );
		material = &context->materialProperty[element->material_I];
		plasticElement = ExtensionManager_Get(  mesh->elementExtensionMgr, element, SnacPlastic_ElementHandle );
		for( tetra_I = 0; tetra_I < Tetrahedra_Count; tetra_I++ ) {
		    plasticElement->plasticStrain[tetra_I] = PlasticStrainFromCohesion(material,(double)contextExt->weakPointCohesion);
#ifdef DEBUG2	    
		    if(tetra_I==0) 
			Journal_Printf( context->snacInfo,"Weak points: timeStep=%d  element (%d,%d,%d)#=%d->%d  cohesion=%g  plasticStrain=%g\n",
					context->timeStep, index_I,index_J,index_K, element_gI, element_lI,  
					contextExt->weakPointCohesion, plasticElement->plasticStrain[tetra_I] );
#endif

		}
	    } // End if
	    
	} // End for
	/*
	 *  Free up allocated memory for shuffling arrays
	 */
	free(shuffleIndexListPtr);
	free(randomNumberListPtr);
    } // End if (check fraction of weak points is in valid range [0,1]


    fprintf(stderr, "CWP trigger pt\n");


    /*
     *  Trigger point assignment
     */
    if(contextExt->triggerPointCohesion>=0.0) {
	ijk[0]=(int)(full_I_element_range*contextExt->xTriggerPointFraction);
	ijk[1]=(int)(full_J_element_range*contextExt->yTriggerPointFraction);
	ijk[1]=	full_J_element_range-1-ijk[1];
	ijk[2]=(int)(full_K_element_range*contextExt->zTriggerPointFraction);
	/*
	 *  Calculate "global" (across all processors) element index and then find local equivalent (this CPU)
	 */
	element_gI = ijk[0] + full_I_element_range*ijk[1] + full_I_element_range*full_J_element_range*ijk[2];
	element_lI = Mesh_ElementMapGlobalToLocal( mesh, element_gI );
	
#ifdef DEBUG
	Journal_Printf(context->snacInfo,  "Trigger point:  ijk = %d,%d,%d  ->  %d  -> %d  <?  %d\n", ijk[0], ijk[1], ijk[2],  element_gI, element_lI, mesh->elementLocalCount);
#endif
	/*
	 *  At trigger point, force low cohesion by imposing a degree of plastic strain
	 */
	if(element_lI < mesh->elementLocalCount) {
	    Snac_Element		*element;
	    Snac_Material	    	*material;
	    SnacPlastic_Element		*plasticElement;
	    Tetrahedra_Index		tetra_I;
	
	    element = Snac_Element_At( context, element_lI );
	    material = &context->materialProperty[element->material_I];
	    plasticElement = ExtensionManager_Get( mesh->elementExtensionMgr,element,SnacPlastic_ElementHandle );
	    /*  Force each of 5*2 tetrahedra to have extra plastic strain to impose lower cohesion indirectly */
	    for( tetra_I = 0; tetra_I < Tetrahedra_Count; tetra_I++ ) {
		plasticElement->plasticStrain[tetra_I] =  PlasticStrainFromCohesion(material,(double)contextExt->triggerPointCohesion);
#ifdef DEBUG
		if(tetra_I==0) Journal_Printf(context->snacInfo, "timeStep=%d ijkt=%d %d %d %d  setting plasticE=%e\n", context->timeStep,ijk[0],ijk[1],ijk[2], tetra_I,  plasticElement->plasticStrain[tetra_I] );
#endif
	    }
	}
    } // End if

    /*
     *  Flag that seeding has been done and don't do any more
     */
    contextExt->seedingCompletedFlag = TRUE;
}




/*
 *----------------------------------------------------------------------
 *
 * PlasticStrainFromCohesion --
 *
 *      Inverts plastic strain from known cohesion 
 *      using material def from Plastic plugin
 *
 * Takes:
 *	material (ptr to struct)
 *	cohesion (double)
 *
 * Returns:
 *	plastic strain (double)
 *
 * Side effects:
 *      Modifies input vectors
 *
 *----------------------------------------------------------------------
 */

double 
PlasticStrainFromCohesion(Snac_Material *material, double cohesion)
{
    int i;
    double plasticStrain=material->plstrain[0];

    //    fprintf(stderr, "In pl\n");
    for( i = 0; i < material->nsegments; i++ ) {
	const double pl1 = material->plstrain[i];
	const double pl2 = material->plstrain[i+1];
	const double coh1 = material->cohesion[i];
	const double coh2 = material->cohesion[i+1];
	plasticStrain = pl1+(pl2-pl1)*( (cohesion-coh1)/(coh2-coh1) );
	if( plasticStrain >= pl1 && plasticStrain <= pl2 ) {
#ifdef DEBUG
	    //	    fprintf(stderr,  "Returning plastic strain = %g  from cohesion = %g  (%g, %g)\n", plasticStrain, cohesion, coh1,coh2);
#endif
	    return plasticStrain;
	}
    }
#ifdef DEBUG
    fprintf(stderr,  "Bailing... default plastic strain = %g  from cohesion = %g \n", plasticStrain, cohesion);
#endif
    return material->plstrain[material->nsegments];
}


/*
 *----------------------------------------------------------------------
 *
 * ShellSort --
 *
 *      Shell sort vector (unsigned int) and coevally shuffle input
 *      (index, unsigned int) vector
 *
 * Returns:
 *	Nothing
 *
 * Side effects:
 *      Modifies input vectors
 *
 *----------------------------------------------------------------------
 */

void 
ShellSort(
	unsigned int vecPtr[],              /* Input unsorted data vector (e.g. random numbers) */  /* was double */
	unsigned int idxPtr[],              /* Input index vector, to be sorted according to vecPtr */
	unsigned long n          	    /* Data length */
	)
{
    unsigned long incr;
    incr=1;
    do {
	incr *= 3;
	incr++;
    } while (incr <= n);
    do {
	unsigned long i,j;
	incr /= 3;
	for (i=incr;i<n;i++) {
	    unsigned int v;  /* was double */
	    unsigned int w;
	    v=vecPtr[i];
	    w=idxPtr[i];
	    j=i;
	    while (vecPtr[j-incr] > v) {
		vecPtr[j]=vecPtr[j-incr];
		idxPtr[j]=idxPtr[j-incr];
		j -= incr;
		if (j < incr) {
		    break;
		}
	    }
	    vecPtr[j]=v;
	    idxPtr[j]=w;
	}
    } while (incr > 1);
}
