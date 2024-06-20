makebin_name = $(1).exe
makelib_name = $(1).lib
makedyn_name = $(1).dll
makeobj_name = $(1).obj
makeres_name = $(1).res
makeres_cmd = rc $(addprefix /d, $(DEFINITIONS)) /r /fo$(2) $(1)

host=windows
compiler := msvs

#windows_libraries += oldnames

# installable boost names convention used
# [prefix]boost_[lib]-mt[-gd].lib
# prefix - 'lib' for static libraries
# lib - library name
# -gd - used for debug libraries
windows_libraries += $(foreach lib,$(libraries.boost),$(if $(boost_dynamic),,lib)boost_$(lib)-mt$(if $(release),,-gd))

# buildable qt names convention used
# Qt[lib][d][4].lib
# lib - library name
# d - used for debug libraries
# 4 - used for dynamic linkage
windows_libraries += $(foreach lib,$(if $(libraries.qt),$(libraries.qt) main,),Qt$(lib)$(if $(release),,d)$(if $(qt_dynamic),4,))
