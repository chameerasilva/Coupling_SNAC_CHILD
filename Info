This repository is going to couple Des3D "https://github.com/echoi/DynEarthSol" with CHILD "https://github.com/childmodel/child" using Creating a Child Interface to Update the DESD

Step 01

=======
1 look into the https://github.com/chameerasilva/Coupling_SNAC_CHILD/blob/master/SNAC_CHILD/Snac/libSnac/src/Context.c for identifying 'child' key word to check how the child interface will access SNAC to update nodes

line 588 --> #if HAVE_CHILD 
line 589 --> #else
line 590 --> line 660 (implementation)

EntryPoint_Append(
		Context_GetEntryPoint( self, Snac_EP_UpdateNodeMomentum ),
		"Snac_DiffTopo",
		Snac_UpdateNodeMomentum_DiffTopo,
		Snac_Context_Type );
	EntryPoint_Append(
		Context_GetEntryPoint( self, Snac_EP_UpdateNodeMomentum ),
		"Snac_DiffTopo",
		Snac_UpdateNodeMomentum_UpdateInteriorTopo,
		Snac_Context_Type );
	EntryPoint_Append(
		Context_GetEntryPoint( self, Snac_EP_UpdateNodeMomentum ),
		"Snac_DiffTopo",
		Snac_UpdateNodeMomentum_AdjustEdgeTopo,
		Snac_Context_Type );
	/*CCCCC*/

These functions are implemented at https://github.com/chameerasilva/Coupling_SNAC_CHILD/blob/master/SNAC_CHILD/Snac/libSnac/src/UpdateNode.c 

UpdateNodeMomentum_DiffTopo || UpdateNodeMomentum_UpdateInteriorTopo || UpdateNodeMomentum_AdjustEdgeTopo


Step 02 
==================
cd childInterface  (make) -->{ libchildInterface.so}

{error 01} in line 2177 in ../child/Code/ChildInterface/../tMesh/tMesh.cpp:

no match for ‘operator<<’ (operand types are ‘std::basic_ostream<char>’ and ‘std::ifstream’ {aka ‘std::basic_ifstream<char>’})
	std::cout << "finished reading file," << gridfile << std::endl;
      |   ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ ^~ ~~~~~~~~
      |             |                              |
      |             std::basic_ostream<char>       std::ifstream {aka std::basic_ifstream<char>}

{solution}
	replace gridfile with arcgridfilenm
	
{error 02}  ../child/Code/ChildInterface/../tMesh/tMesh.cpp:4447:19: error: ordered comparison of pointer with integer zero (‘tEdge*’ and ‘int’)
 4447 |   assert( bndyedg > 0 );
      |           ~~~~~~~~^~~

{solution}  assert( bndyedg > 0 ) --> assert( bndyedg != nullptr )
	
{error 03}../child/Code/ChildInterface/../Erosion/erosion.h:1003:20: error: ordered comparison of pointer with integer zero (‘tPtrList<tLNode>*’ and ‘int’)
   1003 |   if( slideCluster > 0 ) delete slideCluster;
to 1009 same repeating error 
	
{solution} ( slideCluster > 0 ) ---> ( slideCluster != nullptr )
	
{error 03} ../child/Code/ChildInterface/../Erosion/erosion.h:1072:40: error: ordered comparison of pointer with integer zero (‘tEdge*’ and ‘int’)
 1072 | {return ( DF->getAtPtr()->getFlowEdg() > 0 &&

{solution} [DF->getAtPtr()->getFlowEdg() > 0 &&]  --> [DF->getAtPtr()->getFlowEdg() != nullptr &&] 

{errors} in childInterface.cpp
	
	g++ -g -fPIC -I. -I../child/Code/ChildInterface -c childInterface.cpp
childInterface.cpp: In member function ‘double childInterface::RunOneStorm()’:
childInterface.cpp:1041:14: error: ordered comparison of pointer with integer zero (‘tLOutput<tLNode>*’ and ‘int’)
 1041 |   if( output > 0 && time->CheckOutputTime() )
      |       ~~~~~~~^~~
childInterface.cpp:1044:14: error: ordered comparison of pointer with integer zero (‘tLOutput<tLNode>*’ and ‘int’)
 1044 |   if( output > 0 && output->OptTSOutput() ) output->WriteTSOutput();
      |       ~~~~~~~^~~
childInterface.cpp: In member function ‘double childInterface::GetXCoordinate(int, int)’:
childInterface.cpp:1358:15: error: ordered comparison of pointer with integer zero (‘tLNode*’ and ‘int’)
 1358 |    if( my_node<=0 ) return( -9999 );   // Temporary hacked NODATA code
      |        ~~~~~~~^~~
childInterface.cpp: In member function ‘double childInterface::GetYCoordinate(int, int)’:
childInterface.cpp:1384:15: error: ordered comparison of pointer with integer zero (‘tLNode*’ and ‘int’)
 1384 |    if( my_node<=0 ) return( -9999 );   // Temporary hacked NODATA code
      |        ~~~~~~~^~~
childInterface.cpp: In member function ‘double childInterface::GetZCoordinate(int, int)’:
childInterface.cpp:1415:15: error: ordered comparison of pointer with integer zero (‘tLNode*’ and ‘int’)
 1415 |    if( my_node<=0 ) return( -9999 );   // Temporary hacked NODATA code
      |        ~~~~~~~^~~

{solutions}    if( output > 0 && time->CheckOutputTime() )     --->      ( output != nullptr && time->CheckOutputTime() )
			 
{error04} 
	../child/Code/ChildInterface/../tMesh/tMesh.cpp:2927:22: error: ordered comparison of pointer with integer zero (‘tLNode*’ and ‘int’)
 2927 |           assert( cn > 0 );
      |                   ~~~^~~

{solution}  assert( cn > 0 );  -->   assert( cn != nullptr );
			 
====================================================== there are other erros to debug =======================================
			 
Step 03         
===========
cd ../CHILD (make) -->{ libchild.so}
	
{error 01}  
	
../Code/Erosion/erosion.cpp:2925:16: error: ordered comparison of pointer with integer zero (‘tLNode*’ and ‘int’)
 2925 |       assert( n>0 );
      |               ~^~
{Solution}  assert( n>0 ) --> assert( n != nullptr );
	
{error 02} 
	
	../Code/Erosion/erosion.cpp:3167:11: error: ordered comparison of pointer with integer zero (‘tTriangle*’ and ‘int’)
 3167 |     if( gt>0 )

{Solution}   if( gt>0 ) -->  if( gt != nullptr )
	
{error 03} 
	../Code/Erosion/erosion.cpp:3469:36: error: ordered comparison of pointer with integer zero (‘tTrees*’ and ‘int’)
 3469 |   if( cn->getVegCover().getTrees() > 0 )
	
{Solution}   if( cn->getVegCover().getTrees() > 0 ) -->   if( cn->getVegCover().getTrees() != nullptr )
	     if( output > 0 && output->OptTSOutput() ) -->


{error 04}  
	../Code/Erosion/erosion.cpp:3536:15: error: ordered comparison of pointer with integer zero (‘tTriangle*’ and ‘int’)
 3536 |            ct > 0
{solution}    ct > 0  -->  ct != nullptr 
	
{error 05}  
	../Code/Erosion/erosion.cpp:3947:14: error: ordered comparison of pointer with integer zero (‘tDF_RunOut*’ and ‘int’)
 3947 |   if( runout > 0 ) delete runout;
 3948 |   if( scour > 0 ) delete scour;
 3949 |   if( deposit > 0 ) delete deposit;
 3950 |   if( DF_fsPtr > 0 ) delete DF_fsPtr;
 3951 |   if( DF_Hyd_fsPtr > 0 ) delete DF_Hyd_fsPtr;

{solution}  if( runout > 0 ) -->  if( runout != nullptr ) 
	    if( scour > 0 ) -->  if( scour != nullptr )
	    if( deposit > 0 ) --> if( deposit != nullptr )
	    if( DF_fsPtr > 0 ) --> if( DF_fsPtr != nullptr )
	    if( DF_Hyd_fsPtr > 0 ) --> if( DF_Hyd_fsPtr != nullptr )
	
{error 06}  
	../Code/Erosion/erosion.cpp:6261:17: error: ordered comparison of pointer with integer zero (‘tLayer*’ and ‘int’)
 6261 |       if( soilP > 0 )

{solution}   if( soilP > 0 ) -->  if( soilP != nullptr ) 
	
{error 07}
	../Code/Erosion/erosion.cpp:6389:40: error: ordered comparison of pointer with integer zero (‘tTrees*’ and ‘int’)
 6389 |       if( cn->getVegCover().getTrees() > 0 )
	
{solution}  if( cn->getVegCover().getTrees() > 0 ) -->    if( cn->getVegCover().getTrees() != nullptr )
	
{error 08} 
	../Code/Erosion/erosion.cpp:6468:40: error: ordered comparison of pointer with integer zero (‘tTrees*’ and ‘int’)
 6468 |       if( cn->getVegCover().getTrees() > 0 )
      |           ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~^~~
	
{solution}   if( cn->getVegCover().getTrees() > 0 ) -->   if( cn->getVegCover().getTrees() != nullptr )
	
{error 09} 
	../Code/Erosion/erosion.cpp:6786:46: error: ordered comparison of pointer with integer zero (‘tTrees*’ and ‘int’)
 6786 |             if( cn->getVegCover().getTrees() > 0 )
      |                 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~^~~

{solution}   if( cn->getVegCover().getTrees() > 0 ) -->  if( cn->getVegCover().getTrees() != nullptr )
	
{error 10}
	../Code/Erosion/erosion.cpp:6872:50: error: ordered comparison of pointer with integer zero (‘tTrees*’ and ‘int’)
 6872 |                 if( cn->getVegCover().getTrees() > 0 )
      |                     ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~^~~

{solution}   if( cn->getVegCover().getTrees() > 0 ) --> if( cn->getVegCover().getTrees() !=nullptr )
	
{error 11}
	../Code/Erosion/erosion.cpp:6888:46: error: ordered comparison of pointer with integer zero (‘tTrees*’ and ‘int’)
 6888 |             if( cn->getVegCover().getTrees() > 0 )
      |                 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~^~~
{solution}    if( cn->getVegCover().getTrees() > 0 )  --> if( cn->getVegCover().getTrees() != nullptr )
	
{error 12} 
	../Code/Erosion/../tUplift/../tMesh/../tMeshList/tMeshList.h:279:16: error: ‘insertAtNext’ was not declared in this scope, and no declarations were found by argument-dependent lookup at the point of instantiation [-fpermissive]
  279 |    insertAtNext( value, lastactive );

{Solution}  insertAtNext( value, lastactive ); --> this->insertAtNext( value, lastactive );
	
{error 13}
	../Code/tStreamMeander/../tStreamNet/../tUplift/../tMesh/../tMeshList/tMeshList.h:418:39: error: ‘getListNode’ was not declared in this scope, and no declarations were found by argument-dependent lookup at the point of instantiation [-fpermissive]
  418 |    ListNodeType * mvnode = getListNode( mvnodedata );

{solution} getListNode( mvnodedata ); --> this ->getListNode( mvnodedata );

{error 14} 
	../Code/tStreamNet/tStreamNet.cpp:3203:14: error: ordered comparison of pointer with integer zero (‘tLNode*’ and ‘int’)
 3203 |       if( on > 0 )
      |           ~~~^~~
{Solution}  if( on > 0 ) -->  if( on != nullptr )

{error 15} 
	../Code/tStreamNet/tStreamNet.cpp:3218:25: error: ordered comparison of pointer with integer zero (‘tLNode*’ and ‘int’)
 3218 |             } while( dn > 0 && dn->getBoundaryFlag() == kNonBoundary );
      |                      ~~~^~~

{solution}  while( dn > 0 && dn->getBoundaryFlag() == kNonBoundary ); --> while( dn != nullptr && dn->getBoundaryFlag() == kNonBoundary );
	
{error 16}
	../Code/globalFns.cpp:1002:22: error: ordered comparison of pointer with integer zero (‘tLNode*’ and ‘int’)
 1002 |           if( nxtPtr > 0 && nxtPtr->isNonBoundary() )
      |               ~~~~~~~^~~

{solution}    if( nxtPtr > 0 && nxtPtr->isNonBoundary() ) -->  if( nxtPtr != nullptr && nxtPtr->isNonBoundary() )
	
{error 17} 
	../Code/globalFns.cpp:1030:22: error: ordered comparison of pointer with integer zero (‘tLNode*’ and ‘int’)
 1030 |           if( nxtPtr > 0 && nxtPtr->isNonBoundary() )
      |               ~~~~~~~^~~

{solution}    if( nxtPtr > 0 && nxtPtr->isNonBoundary() ) -->   if( nxtPtr != nullptr && nxtPtr->isNonBoundary() )
	
{error 18} 
	../Code/tVegetation/tVegetation.cpp:576:14: error: ordered comparison of pointer with integer zero (‘tTriangle*’ and ‘int’)
  576 |       if( nt > 0 )

{solution}   if( nt > 0 ) -->  if( nt != nullptr )
	
**undefined reference to `predicate'** {bunch of these printed out }
--------------------------------------------------------------------------------------------------------------------------------------------------------	
	g++ -pedantic -Wall -W -Wwrite-strings -Wpointer-arith -Wcast-qual -Wcast-align -Wold-style-cast -g  -O2  -o libchild.  erosion.o meshElements.o mathutil.o tInputFile.o tLNode.o tRunTimer.o tStreamMeander.o meander.o tStorm.o tStreamNet.o tUplift.o errors.o tFloodplain.o tEolian.o globalFns.o predicates.o tVegetation.o tListInputData.o tStratGrid.o tOption.o tTimeSeries.o ParamMesh_t.o TipperTriangulator.o TipperTriangulatorError.o tIDGenerator.o tWaterSedTracker.o tLithologyManager.o 
/usr/bin/ld: /usr/lib/gcc/x86_64-linux-gnu/11/../../../x86_64-linux-gnu/Scrt1.o: in function `_start':
(.text+0x1b): undefined reference to `main'
/usr/bin/ld: erosion.o: in function `tMesh<tLNode>::LocateTriangle(double, double, bool)':
/home/chameera/Coupling_SNAC_CHILD/child/bin/../Code/Erosion/../tUplift/../tMesh/tMesh.cpp:4122: undefined reference to `predicate'
/usr/bin/ld: meshElements.o: in function `tTriangle::NbrToward(double, double)':
/home/chameera/Coupling_SNAC_CHILD/child/bin/../Code/MeshElements/meshElements.cpp:1004: undefined reference to `predicate'
/usr/bin/ld: meshElements.o: in function `tTriangle::containsPoint(double, double) const':
/home/chameera/Coupling_SNAC_CHILD/child/bin/../Code/MeshElements/meshElements.cpp:1029: undefined reference to `predicate'
/usr/bin/ld: globalFns.o: in function `TriPasses(tArray<double> const&, tArray<double> const&, tArray<double> const&, tArray<double> const&)':
/home/chameera/Coupling_SNAC_CHILD/child/bin/../Code/globalFns.cpp:105: undefined reference to `predicate'
/usr/bin/ld: globalFns.o: in function `PointsCCW(tArray<double> const&, tArray<double> const&, tArray<double> const&)':
/home/chameera/Coupling_SNAC_CHILD/child/bin/../Code/globalFns.cpp:174: undefined reference to `predicate'
/usr/bin/ld: globalFns.o:/home/chameera/Coupling_SNAC_CHILD/child/bin/../Code/globalFns.cpp:192: more undefined references to `predicate' follow
collect2: error: ld returned 1 exit status
make: *** [Makefile:34: libchild.] Error 1
-------------------------------------------------------------------------------------------------------------------------------------------------------
