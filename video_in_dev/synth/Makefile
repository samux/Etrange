# La liste des modules nécessaires à la synthèse
include soc_modules.mk
# Le nom du module de niveau TOP
export DESIGN=video_in
export HDL_SEARCH_PATH=../hdl
export SOC_MODULES_FILES = $(foreach module, $(SOC_MODULES), $(shell find ../hdl -regex ".*/$(module)\.[s]?v"))
.PHONY: synth

syn_gui:
	@rm -rf rc.*
	rc  -gui -files rc_synth.tcl -cmdfile rc.cmd -logfile rc.log
syn:
	@rm -rf rc.*
	rc  -files rc_synth.tcl -cmdfile rc.cmd -logfile rc.log
clean:
	@rm -rf rc.*
