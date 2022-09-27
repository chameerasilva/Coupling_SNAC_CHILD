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
