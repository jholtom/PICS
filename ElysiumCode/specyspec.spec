*asm:
--traditional-format -mP %{mcpu=*:-mcpu=%*}%{!mcpu=*:%{mmcu=*:-mmcu=%*}} %{mrelax=-mQ} %{mlarge:-ml} %{!msim:-md} %{msim:%{mlarge:-md}} %{msilicon-errata=*:-msilicon-errata=%*} %{msilicon-errata-warn=*:-msilicon-errata-warn=%*} %{ffunction-sections:-gdwarf-sections} 

*asm_debug:


*asm_final:
%{gsplit-dwarf: 
       objcopy --extract-dwo 	 %{c:%{o*:%*}%{!o*:%b%O}}%{!c:%U%O} 	 %{c:%{o*:%:replace-extension(%{o*:%*} .dwo)}%{!o*:%b.dwo}}%{!c:%b.dwo} 
       objcopy --strip-dwo 	 %{c:%{o*:%*}%{!o*:%b%O}}%{!c:%U%O}     }

*asm_options:
%{-target-help:%:print-asm-header()}  %{gz*:%e-gz is not supported in this configuration} %a %Y %{c:%W{o*}%{!o*:-o %w%b%O}}%{!c:-o %d%w%u%O}

*invoke_as:
%{!fwpa*:   %{fcompare-debug=*|fdump-final-insns=*:%:compare-debug-dump-opt()}   %{!S:-o %|.s |
 as %(asm_options) %m.s %A }  }

*cpp:


*cpp_options:
%(cpp_unique_options) %1 %{m*} %{std*&ansi&trigraphs} %{W*&pedantic*} %{w} %{f*} %{g*:%{!g0:%{g*} %{!fno-working-directory:-fworking-directory}}} %{O*} %{undef} %{save-temps*:-fpch-preprocess}

*cpp_debug_options:
%{d*}

*cpp_unique_options:
%{!Q:-quiet} %{nostdinc*} %{C} %{CC} %{v} %{I*&F*} %{P} %I %{MD:-MD %{!o:%b.d}%{o*:%.d%*}} %{MMD:-MMD %{!o:%b.d}%{o*:%.d%*}} %{M} %{MM} %{MF*} %{MG} %{MP} %{MQ*} %{MT*} %{!E:%{!M:%{!MM:%{!MT:%{!MQ:%{MD|MMD:%{o*:-MQ %*}}}}}}} %{remap} %{g3|ggdb3|gstabs3|gcoff3|gxcoff3|gvms3:-dD} %{!iplugindir*:%{fplugin*:%:find-plugindir()}} %{H} %C %{D*&U*&A*} %{i*} %Z %i %{E|M|MM:%W{o*}}

*trad_capable_cpp:
cc1 -E %{traditional|traditional-cpp:-traditional-cpp}

*cc1:


*cc1_options:
%{pg:%{fomit-frame-pointer:%e-pg and -fomit-frame-pointer are incompatible}} %{!iplugindir*:%{fplugin*:%:find-plugindir()}} %1 %{!Q:-quiet} %{!dumpbase:-dumpbase %B} %{d*} %{m*} %{aux-info*} %{fcompare-debug-second:%:compare-debug-auxbase-opt(%b)}  %{!fcompare-debug-second:%{c|S:%{o*:-auxbase-strip %*}%{!o*:-auxbase %b}}}%{!c:%{!S:-auxbase %b}}  %{g*} %{O*} %{W*&pedantic*} %{w} %{std*&ansi&trigraphs} %{v:-version} %{pg:-p} %{p} %{f*} %{undef} %{Qn:-fno-ident} %{Qy:} %{-help:--help} %{-target-help:--target-help} %{-version:--version} %{-help=*:--help=%*} %{!fsyntax-only:%{S:%W{o*}%{!o*:-o %b.s}}} %{fsyntax-only:-o %j} %{-param*} %{coverage:-fprofile-arcs -ftest-coverage}

*cc1plus:


*link_gcc_c_sequence:
%G %L %G

*link_ssp:
%{fstack-protector|fstack-protector-all|fstack-protector-strong|fstack-protector-explicit:-lssp_nonshared -lssp}

*endfile:
%{!minrt:crtend.o%s} %{minrt:crtn-minrt.o%s}%{!minrt:crtn.o%s} -lgcc

*link:
%{mrelax:--relax} %{mlarge:%{!r:%{!g:--gc-sections}}}

*lib:
					--start-group						%{mhwmult=auto:%{mmcu=*:%:msp430_hwmult_lib(mcu %{mmcu=*:%*});:%:msp430_hwmult_lib(default)};   mhwmult=*:%:msp430_hwmult_lib(hwmult %{mhwmult=*:%*});   mmcu=*:%:msp430_hwmult_lib(mcu %{mmcu=*:%*});		  :%:msp430_hwmult_lib(default)}			-lc							-lgcc							-lcrt							%{msim:-lsim}						%{!msim:-lnosys}					--end-group					   	 

*link_gomp:


*libgcc:
-lgcc

*startfile:
%{pg:gcrt0.o%s}%{!pg:%{minrt:crt0-minrt.o%s}%{!minrt:crt0.o%s}} %{!minrt:crtbegin.o%s}

*cross_compile:
1

*version:
6.2.0

*multilib:
. !mcpu=msp430 !mlarge;430 mcpu=msp430 !mlarge;large !mcpu=msp430 mlarge;

*multilib_defaults:


*multilib_extra:


*multilib_matches:
mcpu=430 mcpu=msp430;mmcu=msp430c091 mcpu=msp430;mmcu=msp430c092 mcpu=msp430;mmcu=msp430c111 mcpu=msp430;mmcu=msp430c1111 mcpu=msp430;mmcu=msp430c112 mcpu=msp430;mmcu=msp430c1121 mcpu=msp430;mmcu=msp430c1331 mcpu=msp430;mmcu=msp430c1351 mcpu=msp430;mmcu=msp430c311s mcpu=msp430;mmcu=msp430c312 mcpu=msp430;mmcu=msp430c313 mcpu=msp430;mmcu=msp430c314 mcpu=msp430;mmcu=msp430c315 mcpu=msp430;mmcu=msp430c323 mcpu=msp430;mmcu=msp430c325 mcpu=msp430;mmcu=msp430c412 mcpu=msp430;mmcu=msp430c413 mcpu=msp430;mmcu=msp430e112 mcpu=msp430;mmcu=msp430e313 mcpu=msp430;mmcu=msp430e315 mcpu=msp430;mmcu=msp430e325 mcpu=msp430;mmcu=msp430f110 mcpu=msp430;mmcu=msp430f1101 mcpu=msp430;mmcu=msp430f1101a mcpu=msp430;mmcu=msp430f1111 mcpu=msp430;mmcu=msp430f1111a mcpu=msp430;mmcu=msp430f112 mcpu=msp430;mmcu=msp430f1121 mcpu=msp430;mmcu=msp430f1121a mcpu=msp430;mmcu=msp430f1122 mcpu=msp430;mmcu=msp430f1132 mcpu=msp430;mmcu=msp430f122 mcpu=msp430;mmcu=msp430f1222 mcpu=msp430;mmcu=msp430f123 mcpu=msp430;mmcu=msp430f1232 mcpu=msp430;mmcu=msp430f133 mcpu=msp430;mmcu=msp430f135 mcpu=msp430;mmcu=msp430f155 mcpu=msp430;mmcu=msp430f156 mcpu=msp430;mmcu=msp430f157 mcpu=msp430;mmcu=msp430f2001 mcpu=msp430;mmcu=msp430f2002 mcpu=msp430;mmcu=msp430f2003 mcpu=msp430;mmcu=msp430f2011 mcpu=msp430;mmcu=msp430f2012 mcpu=msp430;mmcu=msp430f2013 mcpu=msp430;mmcu=msp430f2101 mcpu=msp430;mmcu=msp430f2111 mcpu=msp430;mmcu=msp430f2112 mcpu=msp430;mmcu=msp430f2121 mcpu=msp430;mmcu=msp430f2122 mcpu=msp430;mmcu=msp430f2131 mcpu=msp430;mmcu=msp430f2132 mcpu=msp430;mmcu=msp430f2232 mcpu=msp430;mmcu=msp430f2234 mcpu=msp430;mmcu=msp430f2252 mcpu=msp430;mmcu=msp430f2254 mcpu=msp430;mmcu=msp430f2272 mcpu=msp430;mmcu=msp430f2274 mcpu=msp430;mmcu=msp430f412 mcpu=msp430;mmcu=msp430f413 mcpu=msp430;mmcu=msp430f4132 mcpu=msp430;mmcu=msp430f415 mcpu=msp430;mmcu=msp430f4152 mcpu=msp430;mmcu=msp430f417 mcpu=msp430;mmcu=msp430f4250 mcpu=msp430;mmcu=msp430f4260 mcpu=msp430;mmcu=msp430f4270 mcpu=msp430;mmcu=msp430f435 mcpu=msp430;mmcu=msp430f4351 mcpu=msp430;mmcu=msp430f436 mcpu=msp430;mmcu=msp430f4361 mcpu=msp430;mmcu=msp430f437 mcpu=msp430;mmcu=msp430f4371 mcpu=msp430;mmcu=msp430f438 mcpu=msp430;mmcu=msp430f439 mcpu=msp430;mmcu=msp430f477 mcpu=msp430;mmcu=msp430f478 mcpu=msp430;mmcu=msp430f479 mcpu=msp430;mmcu=msp430fe423 mcpu=msp430;mmcu=msp430fe4232 mcpu=msp430;mmcu=msp430fe423a mcpu=msp430;mmcu=msp430fe4242 mcpu=msp430;mmcu=msp430fe425 mcpu=msp430;mmcu=msp430fe4252 mcpu=msp430;mmcu=msp430fe425a mcpu=msp430;mmcu=msp430fe427 mcpu=msp430;mmcu=msp430fe4272 mcpu=msp430;mmcu=msp430fe427a mcpu=msp430;mmcu=msp430fg4250 mcpu=msp430;mmcu=msp430fg4260 mcpu=msp430;mmcu=msp430fg4270 mcpu=msp430;mmcu=msp430fg437 mcpu=msp430;mmcu=msp430fg438 mcpu=msp430;mmcu=msp430fg439 mcpu=msp430;mmcu=msp430fg477 mcpu=msp430;mmcu=msp430fg478 mcpu=msp430;mmcu=msp430fg479 mcpu=msp430;mmcu=msp430fw423 mcpu=msp430;mmcu=msp430fw425 mcpu=msp430;mmcu=msp430fw427 mcpu=msp430;mmcu=msp430fw428 mcpu=msp430;mmcu=msp430fw429 mcpu=msp430;mmcu=msp430g2001 mcpu=msp430;mmcu=msp430g2101 mcpu=msp430;mmcu=msp430g2102 mcpu=msp430;mmcu=msp430g2111 mcpu=msp430;mmcu=msp430g2112 mcpu=msp430;mmcu=msp430g2113 mcpu=msp430;mmcu=msp430g2121 mcpu=msp430;mmcu=msp430g2131 mcpu=msp430;mmcu=msp430g2132 mcpu=msp430;mmcu=msp430g2152 mcpu=msp430;mmcu=msp430g2153 mcpu=msp430;mmcu=msp430g2201 mcpu=msp430;mmcu=msp430g2202 mcpu=msp430;mmcu=msp430g2203 mcpu=msp430;mmcu=msp430g2210 mcpu=msp430;mmcu=msp430g2211 mcpu=msp430;mmcu=msp430g2212 mcpu=msp430;mmcu=msp430g2213 mcpu=msp430;mmcu=msp430g2221 mcpu=msp430;mmcu=msp430g2230 mcpu=msp430;mmcu=msp430g2231 mcpu=msp430;mmcu=msp430g2232 mcpu=msp430;mmcu=msp430g2233 mcpu=msp430;mmcu=msp430g2252 mcpu=msp430;mmcu=msp430g2253 mcpu=msp430;mmcu=msp430g2302 mcpu=msp430;mmcu=msp430g2303 mcpu=msp430;mmcu=msp430g2312 mcpu=msp430;mmcu=msp430g2313 mcpu=msp430;mmcu=msp430g2332 mcpu=msp430;mmcu=msp430g2333 mcpu=msp430;mmcu=msp430g2352 mcpu=msp430;mmcu=msp430g2353 mcpu=msp430;mmcu=msp430g2402 mcpu=msp430;mmcu=msp430g2403 mcpu=msp430;mmcu=msp430g2412 mcpu=msp430;mmcu=msp430g2413 mcpu=msp430;mmcu=msp430g2432 mcpu=msp430;mmcu=msp430g2433 mcpu=msp430;mmcu=msp430g2444 mcpu=msp430;mmcu=msp430g2452 mcpu=msp430;mmcu=msp430g2453 mcpu=msp430;mmcu=msp430g2513 mcpu=msp430;mmcu=msp430g2533 mcpu=msp430;mmcu=msp430g2544 mcpu=msp430;mmcu=msp430g2553 mcpu=msp430;mmcu=msp430g2744 mcpu=msp430;mmcu=msp430g2755 mcpu=msp430;mmcu=msp430g2855 mcpu=msp430;mmcu=msp430g2955 mcpu=msp430;mmcu=msp430l092 mcpu=msp430;mmcu=msp430p112 mcpu=msp430;mmcu=msp430p313 mcpu=msp430;mmcu=msp430p315 mcpu=msp430;mmcu=msp430p315s mcpu=msp430;mmcu=msp430p325 mcpu=msp430;mmcu=msp430tch5e mcpu=msp430;mmcu=rf430frl152h mcpu=msp430;mmcu=rf430frl152h_rom mcpu=msp430;mmcu=rf430frl153h mcpu=msp430;mmcu=rf430frl153h_rom mcpu=msp430;mmcu=rf430frl154h mcpu=msp430;mmcu=rf430frl154h_rom mcpu=msp430;mmcu=msp430c336 mcpu=msp430;mmcu=msp430c337 mcpu=msp430;mmcu=msp430e337 mcpu=msp430;mmcu=msp430f147 mcpu=msp430;mmcu=msp430f1471 mcpu=msp430;mmcu=msp430f148 mcpu=msp430;mmcu=msp430f1481 mcpu=msp430;mmcu=msp430f149 mcpu=msp430;mmcu=msp430f1491 mcpu=msp430;mmcu=msp430f1610 mcpu=msp430;mmcu=msp430f1611 mcpu=msp430;mmcu=msp430f1612 mcpu=msp430;mmcu=msp430f167 mcpu=msp430;mmcu=msp430f168 mcpu=msp430;mmcu=msp430f169 mcpu=msp430;mmcu=msp430f423 mcpu=msp430;mmcu=msp430f423a mcpu=msp430;mmcu=msp430f425 mcpu=msp430;mmcu=msp430f425a mcpu=msp430;mmcu=msp430f427 mcpu=msp430;mmcu=msp430f427a mcpu=msp430;mmcu=msp430f447 mcpu=msp430;mmcu=msp430f448 mcpu=msp430;mmcu=msp430f4481 mcpu=msp430;mmcu=msp430f449 mcpu=msp430;mmcu=msp430f4491 mcpu=msp430;mmcu=msp430p337 mcpu=msp430;mmcu=msp430afe221 mcpu=msp430;mmcu=msp430afe222 mcpu=msp430;mmcu=msp430afe223 mcpu=msp430;mmcu=msp430afe231 mcpu=msp430;mmcu=msp430afe232 mcpu=msp430;mmcu=msp430afe233 mcpu=msp430;mmcu=msp430afe251 mcpu=msp430;mmcu=msp430afe252 mcpu=msp430;mmcu=msp430afe253 mcpu=msp430;mmcu=msp430f233 mcpu=msp430;mmcu=msp430f2330 mcpu=msp430;mmcu=msp430f235 mcpu=msp430;mmcu=msp430f2350 mcpu=msp430;mmcu=msp430f2370 mcpu=msp430;mmcu=msp430f2410 mcpu=msp430;mmcu=msp430f247 mcpu=msp430;mmcu=msp430f2471 mcpu=msp430;mmcu=msp430f248 mcpu=msp430;mmcu=msp430f2481 mcpu=msp430;mmcu=msp430f249 mcpu=msp430;mmcu=msp430f2491 mcpu=msp430;mmcu=msp430i2020 mcpu=msp430;mmcu=msp430i2021 mcpu=msp430;mmcu=msp430i2030 mcpu=msp430;mmcu=msp430i2031 mcpu=msp430;mmcu=msp430i2040 mcpu=msp430;mmcu=msp430i2041 mcpu=msp430;mmcu=msp430i2xxgeneric mcpu=msp430;mmcu=msp430f4783 mcpu=msp430;mmcu=msp430f4784 mcpu=msp430;mmcu=msp430f4793 mcpu=msp430;mmcu=msp430f4794 mcpu=msp430;mcpu=msp430 mcpu=msp430;mlarge mlarge;

*multilib_exclusions:


*multilib_options:
mcpu=msp430 mlarge

*multilib_reuse:


*linker:
collect2

*linker_plugin_file:


*lto_wrapper:


*lto_gcc:


*post_link:


*link_libgcc:
%D

*md_exec_prefix:


*md_startfile_prefix:


*md_startfile_prefix_1:


*startfile_prefix_spec:


*sysroot_spec:
--sysroot=%R

*sysroot_suffix_spec:


*sysroot_hdrs_suffix_spec:


*self_spec:


*link_command:
%{!fsyntax-only:%{!c:%{!M:%{!MM:%{!E:%{!S:    %(linker) %{!fno-use-linker-plugin:%{!fno-lto:     -plugin %(linker_plugin_file)     -plugin-opt=%(lto_wrapper)     -plugin-opt=-fresolution=%u.res     %{!nostdlib:%{!nodefaultlibs:%:pass-through-libs(%(link_gcc_c_sequence))}}     }}%{flto|flto=*:%<fcompare-debug*}     %{flto} %{fno-lto} %{flto=*} %l %{no-pie:} %{pie:-pie} %{fuse-ld=*:-fuse-ld=%*}  %{gz*:%e-gz is not supported in this configuration} %X %{o*} %{e*} %{N} %{n} %{r}    %{s} %{t} %{u*} %{z} %{Z} %{!nostdlib:%{!nostartfiles:%S}}     %{static:} %{L*} %(mfwrap) %(link_libgcc) %{!nostdlib:%{fvtable-verify=std: -lvtv -u_vtable_map_vars_start -u_vtable_map_vars_end}    %{fvtable-verify=preinit: -lvtv -u_vtable_map_vars_start -u_vtable_map_vars_end}} %{!nostdlib:%{!nodefaultlibs:%{%:sanitize(address):}     %{%:sanitize(thread):}     %{%:sanitize(leak):}}} %o      %{fopenacc|fopenmp|%:gt(%{ftree-parallelize-loops=*:%*} 1):	%:include(libgomp.spec)%(link_gomp)}    %{fcilkplus:%:include(libcilkrts.spec)%(link_cilkrts)}    %{fgnu-tm:%:include(libitm.spec)%(link_itm)}    %(mflib)  %{fsplit-stack: --wrap=pthread_create}    %{fprofile-arcs|fprofile-generate*|coverage:-lgcov} %{!nostdlib:%{!nodefaultlibs:%{%:sanitize(address):%{static-libasan:-Bstatic} -lasan %{static-libasan:-Bdynamic} %{static-libasan:%:include(libsanitizer.spec)%(link_libasan)}    %{static:%ecannot specify -static with -fsanitize=address}}    %{%:sanitize(thread):%{static-libtsan:-Bstatic} -ltsan %{static-libtsan:-Bdynamic} %{static-libtsan:%:include(libsanitizer.spec)%(link_libtsan)}    %{static:%ecannot specify -static with -fsanitize=thread}}    %{%:sanitize(undefined):%{static-libubsan:-Bstatic} -lubsan %{static-libubsan:-Bdynamic} %{static-libubsan:%:include(libsanitizer.spec)%(link_libubsan)}}    %{%:sanitize(leak):%{static-liblsan:-Bstatic} -llsan %{static-liblsan:-Bdynamic} %{static-liblsan:%:include(libsanitizer.spec)%(link_liblsan)}}}}     %{!nostdlib:%{!nodefaultlibs:%(link_ssp) %(link_gcc_c_sequence)}}    %{!nostdlib:%{!nostartfiles:%E}} %{T*}  
%(post_link) }}}}}}

