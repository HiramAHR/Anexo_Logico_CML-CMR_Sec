################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
Aplicacion/source/%.obj: ../Aplicacion/source/%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: ARM Compiler'
	"C:/ti/ccs1200/ccs/tools/compiler/ti-cgt-arm_20.2.6.LTS/bin/armcl" -mv7R4 --code_state=32 --float_support=VFPv3D16 --include_path="C:/Users/ruizca1/Desktop/GOMA/CML_CMR/codigos/Secuenciales/Anexo_Logico_CML_CMR_Sec" --include_path="C:/Users/ruizca1/Desktop/GOMA/CML_CMR/codigos/Secuenciales/Anexo_Logico_CML_CMR_Sec/include" --include_path="C:/ti/ccs1200/ccs/tools/compiler/ti-cgt-arm_20.2.6.LTS/include" -g --diag_warning=225 --diag_wrap=off --display_error_number --enum_type=packed --abi=eabi --preproc_with_compile --preproc_dependency="Aplicacion/source/$(basename $(<F)).d_raw" --obj_directory="Aplicacion/source" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


