
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
**
** Copyright (C), 2003, Victorian Partnership for Advanced Computing (VPAC) Ltd, 110 Victoria Street, Melbourne, 3053, Australia.
**
** Authors:
**	Stevan M. Quenette, Senior Software Engineer, VPAC. (steve@vpac.org)
**	Patrick D. Sunter, Software Engineer, VPAC. (pds@vpac.org)
**	Luke J. Hodkinson, Computational Engineer, VPAC. (lhodkins@vpac.org)
**	Siew-Ching Tan, Software Engineer, VPAC. (siew@vpac.org)
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
** $Id: testLinkedList.c 2136 2005-02-28 02:47:13Z RaquibulHassan $
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include "Base/Foundation/Foundation.h"
#include "Base/IO/IO.h"
#include "Base/Container/Container.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define NUM_DATA 100

int compareFunction(void *data1, void *data2)
{
	int *d1 = NULL, *d2 = NULL;

	d1 = (int*)data1;
	d2 = (int*)data2;

	if (d1 == NULL || d2 == NULL){
		return 0;	
	}
	
	if (*d1 > *d2){
		return  1;
	}
	else if (*d1 == *d2){
		return 0;
	}
	else{
		return -1;
	}
}

void dataPrintFunction( void *nodeData, void *args )
{
	Stream *myStream = NULL;
	
	assert (nodeData);
	myStream = Journal_Register( InfoStream_Type, "LinkedListStream" );

	Journal_Printf( myStream, "\t%d\n", *(int*)nodeData );
}

void dataCopyFunction( void **nodeData, void *newData, SizeT dataSize)
{
	*nodeData = Memory_Alloc_Bytes_Unnamed(dataSize, char);
	memset(*nodeData, 0, dataSize);

	memcpy(*nodeData, newData, dataSize);
}
	
int main( int argc, char* argv[] ) {
	MPI_Comm			CommWorld;
	int				rank;
	int				numProcessors;
	int				procToWatch;
	
	LinkedList *numList = NULL;
	int i = 0;
	int *array[NUM_DATA] = {0};
	int *data;
	int *result = NULL;
	float randomNum, randomMax;
	Stream *myStream = NULL;
	
	/* Initialise MPI, get world info */
	MPI_Init( &argc, &argv );
	MPI_Comm_dup( MPI_COMM_WORLD, &CommWorld );
	MPI_Comm_size( CommWorld, &numProcessors );
	MPI_Comm_rank( CommWorld, &rank );

	BaseFoundation_Init( &argc, &argv );
	BaseIO_Init( &argc, &argv );
	BaseContainer_Init( &argc, &argv );

	if( argc >= 2 ) {
		procToWatch = atoi( argv[1] );
	}
	else {
		procToWatch = 0;
	}
	
	if( rank == procToWatch ) {
		
		myStream = Journal_Register( InfoStream_Type, "LinkedListStream" );
		numList = LinkedList_New(
					compareFunction,
					dataCopyFunction,
					dataPrintFunction,
					NULL,
					LINKEDLIST_UNSORTED);

		Journal_Printf( myStream, "Inserting data into the List\n");
		
		for(i=0; i<NUM_DATA; i++){
			array[i] = Memory_Alloc(int, "TestLinkedList_ArrayEntry");
			randomNum = rand();
			randomMax = RAND_MAX;
			*array[i] = NUM_DATA - i;
			LinkedList_InsertNode(numList, (void*)array[i], sizeof(int));
		}
	
		Journal_Printf(myStream, "\nPrinting the list\n");
		Print( (void*)numList, myStream );
		
		Journal_Printf(myStream, "\nDeleting half the nodes previously inserted into the list\n");
		for(i=0; i<NUM_DATA/2; i++){
			LinkedList_DeleteNode(numList, (void*)array[i]);
		}
		
		Journal_Printf(myStream, "\nPrinting the list\n");
		Print( (void*)numList, myStream );
		

		Journal_Printf(myStream, "\nRetrieving the data held in the list as an array\n");
		data = LinkedList_ReturnArray(numList, int);

		Journal_Printf(myStream, "\nPrinting the array just retrieved\n");
		for(i=0; i<numList->nodeCount; i++){
			printf ("arrayData - %d\n", data[i]);
		}
	
		Journal_Printf( myStream, "\nSearching for Node data in the list\n" );
		for(i=0; i<NUM_DATA/4; i++){
			i = data[i];
			if ((result = LinkedList_FindNodeData(numList, &i, int)) != NULL){
				printf ("Looking for - %d, Found..\n", *result);
			}
			else{
				printf ("Failed to find.. %d\n", i);
			}
		}
	}

	Journal_Printf( myStream, "\nDeleting the List\n" );
	Stg_Class_Delete( (void*)numList );
	
	BaseContainer_Finalise();
	BaseIO_Finalise();
	BaseFoundation_Finalise();
	
	/* Close off MPI */
	MPI_Finalize();
	
	return 0; /* success */
}
