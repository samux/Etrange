
#### Include TCL utility scripts..
##include load_etc.tcl
#### Set up

### DESIGN est le nom du module de plus haut niveau à synthétiser
set DESIGN  $env(DESIGN)
set SOC_MODULES_FILES $env(SOC_MODULES_FILES)
set HDL_SEARCH_PATH $env(HDL_SEARCH_PATH)

#### effort de synthèse pour la synthèse générique (high, medium, low)
set SYN_EFF medium

#### effort de synthèse pour la synthèse avec cellules physiques (high, medium, low)
set MAP_EFF medium

### On récupère la date
set DATE [exec date +%m%d.%H%M]
### On stockera les résultats dans le répertoire marqué par la date de synthèse
set _OUTPUTS_PATH outputs_${DATE}

### On definit ou se trouvent les macros TCL spécifiques à TPT
set_attribute script_search_path {.}
### On charge le script TPT de remplacement des blackboxs (ram, rom...)
include scripts/fix_blackbox_cell.tcl
### On définit ou se trouvent les biliothèques de synthèse 
set_attribute lib_search_path {\
	/comelec/softs/opt/opus_kits/HCMOS9GP_CMP_UPDATE/CORE9GPHS_SNPS_AVT_4.1.a/SNPS/bc_1.32V_0C_wc_1.08V_85C/PHS \
	./tpt_asic_rams/lib \ 
	/comelec/softs/opt/opus_kits/HCMOS9GP_CMP_UPDATE/CORE9GPHS_SNPS_AVT_4.1.a/SIGNOFF/common/LEF \
	./tpt_asic_rams/lef \
        . \
        } 

### On définit ou se trouvent les sources de code RTL 
set_attribute hdl_search_path $HDL_SEARCH_PATH

### On définit le modèle statistique de capacités de routage. Pour le moment on prends celui par défaut
### proposé par la biblothèque physique du fondeur
### (default, enclosed, segmented, top)
set_attribute wireload_mode  default  

### On demande au synthétiseur d'être très verbeux (level de 0 à 9)
set_attribute information_level 7 

### On demande au synthétiseur d'être silencieux pour certain messages de lecture de bibliothèques
### qui sont malheuruesement systématiques avec les bibliothèques "Liberty" utilisées...

### Pour des cellules dont la fonction de sortie n'est pas décrite
### Info    : An output library pin lacks a function attribute. [LBR-41]
set_attribute max_print 0 messages/LBR/LBR-41
### Construction "define_cell_area" non reconnue
### Info    : An unsupported construct was detected in this library. [LBR-40]
set_attribute max_print 0 messages/LBR/LBR-40
### Définitions de timing inutiles
### Warning : Timing attributes intrinsic_rise and intrinsic_fall are not expected and will be ignored.
set_attribute max_print 0 messages/LBR/LBR-21
###
### Info    : Both 'pos_unate' and 'neg_unate' timing_sense arcs have been processed. [LBR-162]
set_attribute max_print 0 messages/LBR/LBR-162

### On récupère les bibliothèques "technologiques et géométriques"
#set_attribute lef_library {HCMOS9GP_ROUTING_DRC.lef CORE9GPHS.lef tpt_hcmos9_dpram_DEPTH128_WIDTH12.lef  tpt_hcmos9_dpram_DEPTH512_WIDTH8.lef tpt_hcmos9_dpram_DEPTH128_WIDTH16.lef  tpt_hcmos9_rom_DEPTH16384_WIDTH32.lef tpt_hcmos9_dpram_DEPTH512_WIDTH32.lef }

###     MODIF1
###     Pour chaque RAM rajouter par exemple 
###	"tpt_hcmos9_dpram_DEPTH128_WIDTH12_Worst.lib" \
###     n'oubliez pas le caractère pour la ligne suite...
       
### TODO RAJOUTER LE NOM DU FICHIER RAM (EX CI DESSUS)
set_attribute library { 
	"CORE9GPHS_Worst.lib"  \
        }


### On garde la correspondance Source/Synthèse
set_attribute hdl_track_filename_row_col  true

## On lit le design
read_hdl -define TPT_TOOL_ASIC_SYNTHESIS -sv $SOC_MODULES_FILES


## On demande au synthétiseur de ne pas se soucier des blocs manquants
#set_attribute hdl_infer_unresolved_from_logic_abstract false
#
#### Première analyse du design
elaborate $DESIGN
puts "End of read_hdl and elaborate"
date

###     MODIF2
####    On remplace les boites noires par les vrais macro cellules
###     Pour chaque RAM rajouter une ligne de type 
#
# TODO EXECUTER
#       tpt_fix_blackbox_cells -design $DESIGN -b asic_sync_dpram_blackbox -c tpt_hcmos9_dpram
###  On verifie que le design a bien été corrigé
check_design -unresolved
check_design 
#
#### On défini les contraintes
##
read_sdc synth.sdc
##puts "The number of exceptions is [llength [find /designs/$DESIGN -exception *]]"
##
synthesize -to_mapped -eff $MAP_EFF -no_incr
##
#####  On verifie que le design est sain
report gates
##
###### On sort
####quit
