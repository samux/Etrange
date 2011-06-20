# YM TPT 2009
# Usage example
# The code search in the hierarchy for blackbox instances named "asic_spram_xx_yy" and then instantiate the corresponding physical cell
# inside. This code should be used after elaboration.
# In a Verilog design we can use a call to a parametrized module , for example : 
#     spram #(32,9) spram01 (...);
# After elaboration RC create a blackbox intance named spram_32_9
# After execution of the code, the physical memory "spram_65nm_32_9" is instantiated in the spram_32_9 instance.
# - BlackBox and Cells should have exactly the same input and output pins.

proc tpt_fix_blackbox_cells {args} {

  switch -- [parse_options [calling_proc] {} $args \
    "-design srs The name of the design to consider" design_name \
    "-blackbox_prefix srs The prefix of the name of blackboxes to be fixed" blackbox_prefix \
    "-cell_prefix srs The prefix of the library cells to use" cell_prefix ] {
      -2 {return}
      0 {return -code error}
  }

  # Find the design 
  set design [find / -design $design_name ]
  if {$design eq ""} {
    puts "TPT Error : Unknown design $design_name"
    error "TPT Failed on 'tpt_fix_blackbox_cells'"
  }
  
#  puts "TPT Found design $design"

  #  Search for subdesigns with blackbox_prefix
  set subdesign_list [find -subdesign  $design/*/$blackbox_prefix*]
  foreach subdes $subdesign_list {
    # Computes the corresponding libcell name using cell_prefix and blackbox_suffix
    set i1  [string last  $blackbox_prefix $subdes]
    set i2  [string length $blackbox_prefix]
    set i3  [expr $i1+$i2]
    set suffix [string range $subdes $i3 end]
    # Find a library/cell to use
    set library_cell_name $cell_prefix$suffix 
    set library_cell [find /libraries -libcell $library_cell_name]
    if { [string compare "" $library_cell]} {
      # Get the associated instances
#      puts "TPT   Found subdes : $subdes"
#      puts "TPT   Found library cell : $library_cell"
      set instance_list [get_attr instance $subdes]
#      puts "TPT   Found instance list : $instance_list"
      # Clean resolved and duplicates subdesigns entries
      set unresolved_inst_list {} 
      set upper_subdesign_list {}
      foreach inst $instance_list {
        if { [ get_attribute unresolved $inst ] eq "true" } {
#  	  puts "TPT   Unresolved  Inst : $inst"
          # OK unresolved instance, then get corresponding sub_design
  	  set upper_inst [dirname [dirname $inst]]
  	  if { $upper_inst eq $design } {
  	    # top level : instance = design 
  	    set upper_design $upper_inst 
  	  } else {
  	    # get sub design name from instance property
  	    set upper_design [get_attr subdesign $upper_inst] 
  	  }
#  	  puts "TPT   Upper design : $upper_design"
          # add the instance to the list if the upper design is new
          if { [ lsearch $upper_subdesign_list $upper_design]  < 0 } {
            lappend upper_subdesign_list $upper_design
            lappend unresolved_inst_list $inst
          }
  
        }
      }
#      puts "TPT       Upper Subdesign List : $upper_subdesign_list"
#      puts "TPT       Unresolved inst List : $unresolved_inst_list"

      for {set i 0} {$i < [llength  $unresolved_inst_list]} {incr i} {
        set inst [ lindex $unresolved_inst_list $i]
        set upper_design [ lindex $upper_subdesign_list $i]
#	puts "TPT    Unresolved Inst : $inst"
#	puts "TPT    Upper Subdesign : $upper_design"
	set binst [basename $inst]
#	puts "TPT    binst : $binst"
	# Rename the instance to be replaced
	set oldinst [mv -flexible $inst ${binst}_tmp]
	set newinst [lrange [edit_netlist new_instance -name ${binst} $library_cell_name $upper_design] 0 0]
        puts "TPT     Replaced $binst by $library_cell_name"
	# Reconnect nets from old instance to new cell instance
	set cell_pin_list [find  $newinst -pin *]
	foreach cell_pin $cell_pin_list {
	    set inst_pin [lindex [find $oldinst -pin [basename $cell_pin]] 0] 
	    edit_netlist connect $cell_pin   $inst_pin
	}
	# Delete old instance
        rm $oldinst 
      }  
    } else {
        puts "TPT ERROR no library cell $library_cell_name for unresolved subdesign [basename $subdes]"
    
   }
  }
}

add_command tpt_fix_blackbox_cells "TPT: This command replaces a blackbox cell with an existing corresponding library cell" "TPT"
