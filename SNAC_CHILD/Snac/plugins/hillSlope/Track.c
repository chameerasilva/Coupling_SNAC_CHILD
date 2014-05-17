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
 ** $Id: Track.c $
 **
 **~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include <StGermain/StGermain.h>
#include <StGermain/FD/FD.h>
#include "Snac/Snac.h"
#include "types.h"
#include "Context.h"
#include "Register.h"
#include "Track.h"
#include <string.h>
#include <math.h>

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

//#define DEBUG
#define DEBUG2
//#define DEBUG3
#define DEBUG4

void SnacHillSlope_Track( void* _context ) {
    Snac_Context		*context = (Snac_Context*)_context;
    SnacHillSlope_Context	*contextExt = ExtensionManager_Get(context->extensionMgr,
								   context,
								   SnacHillSlope_ContextHandle );
    Mesh			*mesh = context->mesh;
    MeshLayout			*layout = (MeshLayout*)mesh->layout;
    HexaMD			*decomp = (HexaMD*)layout->decomp;
    BlockGeometry		*geometry = (BlockGeometry*)layout->elementLayout->geometry;
    const int			full_I_node_range=decomp->nodeGlobal3DCounts[0];
    const int			full_J_node_range=decomp->nodeGlobal3DCounts[1];
    const int			full_K_node_range=decomp->nodeGlobal3DCounts[2];
    int				index_I,index_J,index_K;

    static double		*yGridOldPtr, *yGridOlderPtr;
    double			max_yVelocity, max_yAcceln;
    static double		unit_yVelocity=0.0, unit_yAcceln=0.0;
    static double		old_max_yVelocity=0.0, old_max_yAcceln=0.0;
    /* 	static double			min_yVelocity=0.0, min_yAcceln=0.0; */
    static char			fallingFlag=FALSE, doneTrackingFlag=FALSE;
    char			reachesTopFlag=FALSE;

    const double		trackLevel=(double)contextExt->trackLevel;
    const double		startThreshold=(contextExt->startThreshold>=0.0 ? contextExt->startThreshold : 1e-2);
    const double		stopThreshold=(contextExt->stopThreshold>=0.0 ? contextExt->stopThreshold : 1e-3);
    int				maxTimeSteps=context->maxTimeSteps, dumpEvery=context->dumpEvery;
    int				restart = FALSE;
    Dictionary_Entry_Value 	*pluginsList, *plugin;

    /* 	if (context->timeStep % context->dumpEvery == 0) { */
    /* 	    Journal_Printf( context->snacInfo,"timeStep=%d (in track)\n", context->timeStep ); */
    /* 	} */

    /*
     *  Bail now if all threads have reached elastic equilibrium
     */
    if(contextExt->consensusElasticStabilizedFlag || doneTrackingFlag){
#ifdef DEBUG3
	fprintf(stderr,"r=%d, ts=%d/%d: Consensus eqm... bailing at top of Track.c\n",context->rank, 
		context->timeStep,context->maxTimeSteps);
#endif
	return;
    }

    pluginsList = PluginsManager_GetPluginsList( context->dictionary );
    if (pluginsList) {
	plugin = Dictionary_Entry_Value_GetFirstElement(pluginsList);
	while ( plugin ) {
	    if ( 0 == strcmp( Dictionary_Entry_Value_AsString( plugin ),
			      "SnacRestart" ) ) {
		restart = TRUE;
		break;
	    }
	    plugin = plugin->next;
	}
    }
/*     if( restart ) { */
/* 	fprintf(stderr, "Restarting: in Track.c with ts=%d, restart ts=%d\n", context->timeStep, context->restartTimestep); */
/* 	return; */
/*     } */

/*     restart=FALSE; */

    /*
     *  Set up a tracking grids (slices of mesh) to allow t instance to be compared with t-1, t-2 instances
     */
    if(context->timeStep==1 || (restart && (context->timeStep-context->restartTimestep)==1)) {
#ifdef DEBUG4
    fprintf(stderr,"Tracking:  creating surface grid record: %d\n",context->timeStep-context->restartTimestep);
#endif
	yGridOldPtr=(double *)malloc((size_t)(full_I_node_range*full_K_node_range*sizeof(double)));
	yGridOlderPtr=(double *)malloc((size_t)(full_I_node_range*full_K_node_range*sizeof(double)));
	for(index_I = 0; index_I < full_I_node_range; index_I++) {
	    for(index_K = 0; index_K < full_K_node_range; index_K++) {
		*(yGridOlderPtr+index_I + full_I_node_range*index_K)=-1.0E19;
		*(yGridOldPtr+index_I + full_I_node_range*index_K)=-1.0E19;
	    }
	}
    }

    /*
     * Compare t instance with t-1, t-2 instances, calculating slice vel, acceln and identifying maxima
     */
    max_yVelocity=0.0;
    max_yAcceln=0.0;
    for(index_I = 0; index_I < full_I_node_range; index_I++) {
	for(index_K = 0; index_K < full_K_node_range; index_K++) {
	    Node_GlobalIndex		node_gI;
	    Node_LocalIndex			node_lI;
	    Coord				*coordPtr = 0;
	    double				node_yElevation, node_yVelocity, node_yAcceln;
	    double				*tmp_yGridOldPtr, *tmp_yGridOlderPtr;

	    index_J=(int)(((double)full_J_node_range-1.0)*(1.0-trackLevel));
	    node_gI = index_I + full_I_node_range*index_J + full_I_node_range*full_J_node_range*index_K;
	    node_lI = Mesh_NodeMapGlobalToLocal( mesh, node_gI );

	    /* 
	     * If a local node, read its elevation, if not, give a dummy value 
	     */
	    if( node_lI < context->mesh->nodeLocalCount ) { 
		/* 
		 *  A local node 
		 */
		coordPtr = Snac_NodeCoord_P( context, node_lI );
		node_yElevation = (*coordPtr)[1]+(*coordPtr)[1]+(*coordPtr)[1];
		reachesTopFlag |= TRUE;
	    }
	    else {
		/*
		 *  Not a local node - handled by another CPU thread
		 */
		node_yElevation=-1.0E29;
	    }
	    tmp_yGridOldPtr = yGridOldPtr+index_I + full_I_node_range*index_K;
	    tmp_yGridOlderPtr = yGridOlderPtr+index_I + full_I_node_range*index_K;
	    /*
	     * Start assessing the vertical surface motions after two time steps
	     */
	    node_yVelocity = node_yElevation-*tmp_yGridOldPtr;
	    node_yAcceln = node_yVelocity-(*tmp_yGridOldPtr-*tmp_yGridOlderPtr);
	    if(context->timeStep>=3 || (restart && (context->timeStep-context->restartTimestep)>=3)){
		if(fabs(node_yVelocity)>max_yVelocity)
		    max_yVelocity = fabs(node_yVelocity);
		if(fabs(node_yAcceln)>max_yAcceln)
		    max_yAcceln = fabs(node_yAcceln);
	    }
	    /*
	     * Record this elevation field and push previous back to "OlderPtr" array
	     */
	    *tmp_yGridOlderPtr = *tmp_yGridOldPtr;
	    *tmp_yGridOldPtr = node_yElevation;
	}
    }

#ifdef DEBUG2
    if (reachesTopFlag)
    fprintf(stderr,
	    "r=%d, ts=%d/%d:    top?=%d  tracking?=%d  eqm?=%d  consensus?=%d:  max_vel=%g  unit_vel=%g\n",
	    context->rank, context->timeStep, context->maxTimeSteps, reachesTopFlag, 
	    contextExt->startedTrackingFlag, contextExt->elasticStabilizedFlag, contextExt->consensusElasticStabilizedFlag, 
	    max_yVelocity, unit_yVelocity ); 
#endif

    /*
     *  If none of the top-surface nodes are local, set flags to pretend we're equilibrated on this thread
     */
    if(!reachesTopFlag) {
	contextExt->startedTrackingFlag = TRUE;
	contextExt->elasticStabilizedFlag = TRUE;
#ifdef DEBUG
	fprintf(stderr,"r=%d, ts=%d/%d:  Doesn't reach top\n",context->rank, context->timeStep, context->maxTimeSteps);
#endif
    } else {
	/*
	 * Now deprecated: estimate unit rates of motion for later comparison with falling rates
	 */
/* 	if(unit_yVelocity==0.0 && max_yVelocity>0.0) */
/* 	    unit_yVelocity = max_yVelocity;		 */
/* 	if(unit_yAcceln==0.0 && max_yAcceln>0.0) */
/* 	    unit_yAcceln = max_yAcceln;	 */
	if( !contextExt->startedTrackingFlag && max_yVelocity>=startThreshold
	    && (context->timeStep>=4  || (restart && (context->timeStep-context->restartTimestep)>=4)) ) 
	    contextExt->startedTrackingFlag=TRUE;

#ifdef DEBUG
	fprintf(stderr,"r=%d, ts=%d/%d: Does reach top - check if equilibrating: unit_vel=%g\n",
		context->rank, context->timeStep, context->maxTimeSteps, unit_yVelocity);
#endif
	/*
	 *  If surface change is slowing and slowly enough, flag that elastic eqm has been reached on this thread
	 */
	if(contextExt->startedTrackingFlag){
	    fallingFlag = CheckFallingFn(max_yVelocity,max_yAcceln,old_max_yVelocity,old_max_yAcceln);
	    /* 	    if(CheckStabilizingFn(max_yVelocity/unit_yVelocity, max_yAcceln/unit_yAcceln, stopThreshold, fallingFlag)==TRUE */
	    if( !contextExt->elasticStabilizedFlag
		&& CheckStabilizingFn(max_yVelocity, max_yAcceln, stopThreshold, fallingFlag)==TRUE
		&& (context->maxTimeSteps!=context->timeStep  
		    || (restart && (context->maxTimeSteps!=(context->timeStep-context->restartTimestep)) )) ) {
		/*
		 *  Stabilizing on this thread
		 */
#ifdef DEBUG
		fprintf(stderr,"r=%d, ts=%d/%d: Locally report we're equilibrating\n",context->rank, context->timeStep, context->maxTimeSteps);
#endif
		contextExt->elasticStabilizedFlag = TRUE;
	    }
	}
    }
    /*
     *  Decide whether to stop or to continue simulation
     */
#ifdef DEBUG
    fprintf(stderr,"r=%d, ts=%d/%d: Checking consensus... local=%d, global=%d\n",context->rank, context->timeStep, context->maxTimeSteps,
	    contextExt->elasticStabilizedFlag,contextExt->consensusElasticStabilizedFlag);
#endif
    /*
     *  Check all threads to see if global equilibration has been reached
     */
    MPI_Allreduce( &(contextExt->elasticStabilizedFlag), &(contextExt->consensusElasticStabilizedFlag), 
		   1, MPI_CHAR, MPI_MIN, context->communicator );
#ifdef DEBUG
    fprintf(stderr,"r=%d, ts=%d/%d: ... revised consensus, global=%d\n",context->rank, context->timeStep, context->maxTimeSteps,
	    contextExt->consensusElasticStabilizedFlag);
#endif
    /*
     *  Have we reached elastic eqm?
     */
    if(contextExt->consensusElasticStabilizedFlag) {
#ifdef DEBUG4
	fprintf(stderr,"r=%d, ts=%d/%d: Global elastic eqm has been reached\n",
		context->rank, context->timeStep, context->maxTimeSteps);
#endif
	if(contextExt->solveElasticEqmOnlyFlag) {
	    /*
	     * If solving elastic eqm only, stop the simulation by bringing forward the max time steps to this step.
	     * In addition, force a dump of this model state by changing dump freq to 1.
	     */
	    dumpEvery=1;
	    maxTimeSteps=(!restart ? context->timeStep+1 : context->timeStep-context->restartTimestep+1);
	    doneTrackingFlag=TRUE;
#ifdef DEBUG4
	fprintf(stderr,"r=%d, ts=%d/%d: Terminating simulation at step=%d\n",context->rank, context->timeStep, 
		context->maxTimeSteps, maxTimeSteps);
#endif
	} else {
	    /*
	     * Otherwise, change dump frequency to record plastic deformation
	     */
	    dumpEvery=contextExt->plasticDeformationDumpFreq;
#ifdef DEBUG4
	fprintf(stderr,"r=%d, ts=%d/%d: Shifting to elastoplastic simulation at step=%d for %d more steps\n",context->rank, context->timeStep, 
		context->maxTimeSteps, context->timeStep, maxTimeSteps-(context->timeStep-context->restartTimestep));
#endif
	}
    }
    /*
     *  Ensure that all threads are in agreement
     */
    if(contextExt->consensusElasticStabilizedFlag) {
	MPI_Allreduce( &(maxTimeSteps), &(context->maxTimeSteps), 1, MPI_INT, MPI_MIN, context->communicator );
	MPI_Allreduce( &(dumpEvery), &(context->dumpEvery), 1, MPI_INT, MPI_MIN, context->communicator );
	/*
	 *  Update dump parameters to ensure that state dumps occur at the revised frequency
	 */
	_Snac_Context_AdjustDump( context );
/* 	if(dumpEvery==1) { */
/* #ifdef DEBUG */
/* 	    fprintf(stderr,"Track.c:  trying to dump stress tensor\n"); */
/* #endif */
/* 	    _Snac_Context_CalcStresses( context ); */
/* 	    _Snac_Context_DumpStressTensor( context ); */
/* 	} */
    }

#ifdef DEBUG
    fprintf(stderr,"r=%d, ts=%d/%d: Exiting Track.c with dump freq=%d\n",
	    context->rank, context->timeStep, context->maxTimeSteps, context->dumpEvery);
#endif
    /*
     * Record the current mesh slice velocity and acceln for use next iteration
     */
    old_max_yVelocity = max_yVelocity;
    old_max_yAcceln = max_yAcceln;

}


/*
 *----------------------------------------------------------------------
 *
 * CheckFallingFn --
 *
 *      Decide whether solution is heading away from or towards equilibrium
 *
 * Returns:
 *	True=falling/False=rising
 *
 * Side effects:
 *      None
 *
 *----------------------------------------------------------------------
 */

char 
CheckFallingFn(double velocity, double acceln, double old_velocity, double old_acceln)
{
    /*
     *  Return TRUE if either velocity or acceln is dropping or constant 
     */
/*     fprintf(stderr, "%g->%g   %g->%g\n", old_velocity,velocity,old_acceln,acceln); */
    return ( ((velocity<old_velocity) && (acceln<old_acceln)) ? TRUE : FALSE );
}


/*
 *----------------------------------------------------------------------
 *
 * CheckStabilizingFn --
 *
 *      Decide whether solution  is stabilizing
 *
 * Returns:
 *	True=stabilizing/False=still changing
 *
 * Side effects:
 *      None
 *
 *----------------------------------------------------------------------
 */

char 
CheckStabilizingFn(double velocity, double acceln, double stopThreshold, char fallingFlag)
{
    return ( fallingFlag & (velocity<=stopThreshold /* && acceln<stopThreshold */ ? TRUE : FALSE) );
}
