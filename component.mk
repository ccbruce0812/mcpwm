INC_DIRS += $(mcpwm_ROOT)

mcpwm_SRC_DIR = $(mcpwm_ROOT)

$(eval $(call component_compile_rules,mcpwm))
