#path/lang/domain.ts
getlang = $(lastword $(subst /, ,$(dir $(1))))

tools.lupdate ?= $(qt.bin)lupdate
tools.lrelease ?= $(qt.bin)lrelease

%.ts: $(sort $(wildcard $(addsuffix *$(src_suffix),$(sort $(dir $(source_files))))) $(addsuffix *.ui,$(sort $(dir $(ui_files)))))
	$(tools.lupdate) -target-language $(call getlang,$@) $^ -ts $@

.PRECIOUS: %.ts

qm_files = $(foreach lng,$(l10n_languages),$(foreach file,$(ts_files),$(l10n_dir)/$(lng)/$(file).qm))

%.qm: %.ts
	$(tools.lrelease) $^ -qm $@
