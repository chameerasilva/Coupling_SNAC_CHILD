//-*-c++-*-

/**************************************************************************/
/**
**  @file ChildInterface.h
**
**  @brief Header file for the general interface to CHILD.
**
**    This is the header file for ChildInterface, which
**    provides a generalized, OpenMI-style interface to the CHILD
**    landscape evolution model.
**
**  The first version is really just a sketch.
**
**  For information regarding this program, please contact Greg Tucker at:
**
**     Cooperative Institute for Research in Environmental Sciences (CIRES)
**     and Department of Geological Sciences
**     University of Colorado
**     2200 Colorado Avenue, Campus Box 399
**     Boulder, CO 80309-0399
**
*/
/**************************************************************************/

#ifndef CHILDINTERFACE_H
#define CHILDINTERFACE_H

#include <string>
#include <sstream>
#include <vector>
#include "trapfpe.h"
#include "Inclusions.h"
#include "tFloodplain/tFloodplain.h"
#include "tStratGrid/tStratGrid.h"
#include "tEolian/tEolian.h"
#include "tOption/tOption.h"
#include "tWaterSedTracker/tWaterSedTracker.h"
#include "tMeshList/tMeshList.h"
#include "tLithologyManager/tLithologyManager.h"

using namespace std;

// A type definition to implement OpenMI IElement interface
enum ElementType
{
	IDBased,
	XYPoint,
	XYLine,
	XYPolyLine,
	XYPolygon,
	XYZPoint,
	XYZLine,
	XYZPolyLine,
	XYZPolygon,
	XYZPolyhedron
};

//Predicates predicate;

/**************************************************************************/
/**
**  Class childInterface
**
**  The class childInterface acts as a copy of CHILD, providing access
**  to it via methods to Initialize, RunOneStorm, Run (for a given
**  duration as given as an argument or in an input file), and CleanUp.
*/
/**************************************************************************/
class childInterface
{
public:
  // Public methods
  childInterface();
  void Initialize_Copy( const childInterface& );
  void Initialize( int argc, char **argv );
  void Initialize( string argument_string );
  void Initialize( int argc, char **argv, int nox, int noy, double*** pts );
  void Initialize( string argument_string, int nox, int noy, double*** pts );
  void Initialize( string argument_string, const int numpts, std::vector<double *> &pts, std::vector<int> &bmarker );
  vector<double> VaryParameters( const tInputFile &, const double &, tRand &, 
				 bool yesVary = true ); 
  double RunOneStorm();
  void Run( double run_duration );
  void CleanUp();
  ~childInterface();
  void ExternalErosionAndDeposition( vector<double> dz );
  void ExternalErodeAndDepositToElevation( vector<double> z );
  void TrackWaterAndSedFluxAtNodes( vector<int> ids_of_nodes_to_track,
				    double current_time );
	vector<double> GetLoads();
	
  // Public methods that implement the OpenMI IElement interface
  string getID();
  string getDescription();
  //Question: how to handle ISpatialReference? what type is it? do I define such a class?
  ElementType getElementType();
  int getElementCount();
  int getVersion();
  int GetElementIndex( string elementID );
  string GetElementID( int element_index );
  int GetVertexCount( int element_index );
  int GetFaceCount( int element_index );
  vector<int> GetFaceVertexIndices( int element_index, int face_index );
  double GetXCoordinate( int element_index, int vertex_index );
  double GetYCoordinate( int element_index, int vertex_index );
  double GetZCoordinate( int element_index, int vertex_index );
  std::vector<double> GetValueSet( string var_name );
  void SetValueSet( string var_name, std::vector<double> values );
  void setWriteOption( bool, tInputFile& );
  void WriteChildStyleOutput();
  void ChangeOption( string option, int val );
  void ModifyCoords( double*** pts );
  void GetNewCoords( double*** pts );
  void ModifyCoords( const std::vector<double *> &pts );
  void GetNewCoords( std::vector<double *> &pts );

  // Additional custom functions to accompany IElement interface
  bool IsInteriorNode( int element_index );
  long GetNodeCount();
  std::vector<double> GetNodeCoords();
  long GetTriangleCount();
  std::vector<long> GetTriangleVertexIDs();
  double GetCurrentTime();  // Returns current time in simulation
  double GetRemainingRunTime();   // Returns the ending time for the run
  void AdjustElevations( std::vector<double> dz );  // changes elevs
  void AdjustInteriorElevations( std::vector<double> dz );  // changes elevs
	void GetNodeXCoords( std::vector<double> & x );  // returns node x coordinates
	void GetNodeYCoords( std::vector<double> & y );  // returns node y coordinates
	std::vector<double> GetNodeXCoords();  // returns node x coordinates
	std::vector<double> GetNodeYCoords();  // returns node y coordinates
	
  // Interface functions used (at the moment) only for development and testing
  tMesh<tLNode> * GetMeshPointer() { return mesh; }
		
private:

  // Private methods
  std::vector<double> GetNodeElevationVector();  // Creates and returns vector of elevs
  std::vector<double> GetNodeErosionVector();  // Creates and returns vector of ero/dep
  std::vector<double> GetNodeDischargeVector();  // Creates and returns vector of Q
  std::vector<double> GetNodeSedimentFluxVector();  // Creates and returns vector of Qs
  void SetNodeElevations( std::vector<double> elevations );
  std::vector<double> GetLandslideAreasVector(); // Creates and returns vector of landslides
  
  // Private data
  bool initialized;      // Flag indicated whether model has been initialized
  bool optNoDiffusion,   // Option to turn off diffusive processes (default to false)
    optNoFluvial,        // Option to turn off fluvial processes (default to false)
    optNoUplift,       // Option to turn off uplift (default to false) 
    optDetachLim,      // Option for detachment-limited erosion only
    optFloodplainDep,  // Option for floodplain (overbank) deposition
    optLoessDep,       // Option for eolian deposition
    optVegetation,     // Option for dynamic vegetation cover
    optFire,           // Option for fire object within tVegetation
    optForest,         // Option for forest object within tVegetation
    optMeander,        // Option for stream meandering
    optDiffuseDepo,    // Option for deposition / no deposition by diff'n
    optStratGrid,      // Option to enable stratigraphy grid
    optTrackWaterSedTimeSeries,  // Option to record timeseries Q and Qs
    optNonlinearDiffusion, // Option for nonlinear creep transport
    optDepthDependentDiffusion, // Option for depth dependent creep transport
    optLandslides, // Option for landsliding
    opt3DLandslides, // Option for determining which landslide function to use
    optChemicalWeathering, // Option for chemical weathering
    optPhysicalWeathering; // Option for physical weathering
  bool optStreamLineBoundary; // Option for converting streamlines to open boundaries
  tRand *rand;             // -> random number generator
  tMesh<tLNode> *mesh;        // -> mesh object
  tLOutput<tLNode> *output;   // -> output handler
  tStorm *storm;              // -> storm generator
  tStreamNet *strmNet;	    // -> stream network module
  tErosion *erosion;          // -> erosion module
  tUplift *uplift;            // -> uplift/baselevel module
  tRunTimer *time;             // -> run timer
  tWaterSedTracker water_sed_tracker_;   // Water and sediment tracker
	tLithologyManager lithology_manager_;  // Lithology manager
  tVegetation *vegetation;  // -> vegetation object
  tFloodplain *floodplain;  // -> floodplain object
  tStratGrid *stratGrid;     // -> Stratigraphy Grid object
  tEolian *loess;           // -> eolian deposition object
  tStreamMeander *strmMeander; // -> stream meander object
  //Predicates predicate;   // Math-related stuff
	
  // Private data for implementing OpenMI IElement interface
  string element_set_id;
  string element_set_description;
  int version;
};


#endif
