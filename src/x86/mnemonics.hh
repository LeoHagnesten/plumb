#pragma once

#define PLUMB_MNEMONICS( X ) \
	X ( invalid ) \
	X ( aaa ) X ( aad ) X ( aam ) X ( aas ) X ( adc ) X ( adcx ) X ( add ) X ( adox ) X ( and_ ) X ( andn ) X ( arpl ) \
	X ( bextr ) X ( blsi ) X ( blsmsk ) X ( blsr ) X ( bound ) X ( bsf ) X ( bsr ) X ( bswap ) X ( bt ) X ( btc ) X ( btr ) X ( bts ) X ( bzhi ) \
	X ( call ) X ( callf ) X ( cbw ) X ( cwde ) X ( cdqe ) X ( cwd ) X ( cdq ) X ( cqo ) X ( clac ) X ( clc ) X ( cld ) X ( clflush ) X ( clflushopt ) X ( clwb ) X ( cli ) X ( clts ) X ( cmc ) \
	X ( cmpxchg ) X ( cmpxchg8b ) X ( cmpxchg16b ) X ( cmp ) X ( cpuid ) X ( crc32 ) \
	X ( daa ) X ( das ) X ( dec ) X ( div ) X ( emms ) X ( endbr32 ) X ( endbr64 ) X ( enter ) X ( fwait ) X ( getsec ) X ( hlt ) \
	X ( idiv ) X ( imul ) X ( in ) X ( inc ) X ( incsspd ) X ( incsspq ) X ( int_ ) X ( int1 ) X ( int3 ) X ( into ) X ( invd ) X ( invlpg ) X ( invpcid ) X ( iret ) X ( iretd ) X ( iretq ) \
	X ( jo ) X ( jno ) X ( jb ) X ( jae ) X ( je ) X ( jne ) X ( jbe ) X ( ja ) X ( js ) X ( jns ) X ( jp ) X ( jnp ) X ( jl ) X ( jge ) X ( jle ) X ( jg ) \
	X ( jcxz ) X ( jecxz ) X ( jrcxz ) X ( jmp ) X ( jmpf ) \
	X ( lahf ) X ( lar ) X ( lds ) X ( lea ) X ( leave ) X ( les ) X ( lfence ) X ( lfs ) X ( lgdt ) X ( lgs ) X ( lidt ) X ( lldt ) X ( lmsw ) \
	X ( loop ) X ( loope ) X ( loopne ) X ( lsl ) X ( lss ) X ( ltr ) X ( lzcnt ) \
	X ( mfence ) X ( monitor ) X ( mov ) X ( movbe ) X ( movsx ) X ( movsxd ) X ( movzx ) X ( mul ) X ( mulx ) X ( mwait ) \
	X ( neg ) X ( nop ) X ( not_ ) X ( or_ ) X ( out ) X ( pause ) X ( pdep ) X ( pext ) X ( pop ) X ( popa ) X ( popad ) X ( popcnt ) X ( popf ) X ( popfd ) X ( popfq ) \
	X ( prefetch ) X ( prefetchw ) X ( prefetchnta ) X ( prefetcht0 ) X ( prefetcht1 ) X ( prefetcht2 ) \
	X ( push ) X ( pusha ) X ( pushad ) X ( pushf ) X ( pushfd ) X ( pushfq ) \
	X ( rcl ) X ( rcr ) X ( rdfsbase ) X ( rdgsbase ) X ( rdmsr ) X ( rdpid ) X ( rdpkru ) X ( rdpmc ) X ( rdrand ) X ( rdseed ) X ( rdsspd ) X ( rdsspq ) X ( rdtsc ) X ( rdtscp ) \
	X ( ret ) X ( retf ) X ( rol ) X ( ror ) X ( rorx ) X ( rsm ) X ( sahf ) X ( salc ) X ( sar ) X ( sarx ) X ( sbb ) \
	X ( seto ) X ( setno ) X ( setb ) X ( setae ) X ( sete ) X ( setne ) X ( setbe ) X ( seta ) X ( sets ) X ( setns ) X ( setp ) X ( setnp ) X ( setl ) X ( setge ) X ( setle ) X ( setg ) \
	X ( sfence ) X ( sgdt ) X ( shl ) X ( shld ) X ( shlx ) X ( shr ) X ( shrd ) X ( shrx ) X ( sidt ) X ( sldt ) X ( smsw ) X ( stac ) X ( stc ) X ( std ) X ( sti ) X ( str ) X ( sub ) \
	X ( swapgs ) X ( syscall ) X ( sysenter ) X ( sysexit ) X ( sysret ) X ( test ) X ( tzcnt ) X ( ud0 ) X ( ud1 ) X ( ud2 ) X ( verr ) X ( verw ) \
	X ( vmcall ) X ( vmlaunch ) X ( vmresume ) X ( vmxoff ) X ( vmread ) X ( vmwrite ) X ( wbinvd ) X ( wrfsbase ) X ( wrgsbase ) X ( wrmsr ) X ( wrpkru ) \
	X ( xabort ) X ( xadd ) X ( xbegin ) X ( xchg ) X ( xend ) X ( xgetbv ) X ( xlatb ) X ( xor_ ) X ( xrstor ) X ( xsave ) X ( xsaveopt ) X ( xsetbv ) X ( xtest ) \
	X ( fxrstor ) X ( fxsave ) X ( ldmxcsr ) X ( stmxcsr ) X ( xrstors ) X ( xsavec ) X ( xsaves ) \
	X ( cmovo ) X ( cmovno ) X ( cmovb ) X ( cmovae ) X ( cmove ) X ( cmovne ) X ( cmovbe ) X ( cmova ) X ( cmovs ) X ( cmovns ) X ( cmovp ) X ( cmovnp ) X ( cmovl ) X ( cmovge ) X ( cmovle ) X ( cmovg ) \
	X ( movsb ) X ( movsw ) X ( movsd_ ) X ( movsq ) X ( cmpsb ) X ( cmpsw ) X ( cmpsd_ ) X ( cmpsq ) X ( stosb ) X ( stosw ) X ( stosd ) X ( stosq ) \
	X ( lodsb ) X ( lodsw ) X ( lodsd ) X ( lodsq ) X ( scasb ) X ( scasw ) X ( scasd ) X ( scasq ) X ( insb ) X ( insw ) X ( insd ) X ( outsb ) X ( outsw ) X ( outsd ) \
	X ( f2xm1 ) X ( fabs ) X ( fadd ) X ( faddp ) X ( fbld ) X ( fbstp ) X ( fchs ) X ( fcmovb ) X ( fcmovbe ) X ( fcmove ) X ( fcmovnb ) X ( fcmovnbe ) X ( fcmovne ) X ( fcmovnu ) X ( fcmovu ) \
	X ( fcom ) X ( fcomi ) X ( fcomip ) X ( fcomp ) X ( fcompp ) X ( fcos ) X ( fdecstp ) X ( fdiv ) X ( fdivp ) X ( fdivr ) X ( fdivrp ) X ( ffree ) X ( ffreep ) \
	X ( fiadd ) X ( ficom ) X ( ficomp ) X ( fidiv ) X ( fidivr ) X ( fild ) X ( fimul ) X ( fincstp ) X ( fist ) X ( fistp ) X ( fisttp ) X ( fisub ) X ( fisubr ) \
	X ( fld ) X ( fld1 ) X ( fldcw ) X ( fldenv ) X ( fldl2e ) X ( fldl2t ) X ( fldlg2 ) X ( fldln2 ) X ( fldpi ) X ( fldz ) X ( fmul ) X ( fmulp ) \
	X ( fnclex ) X ( fninit ) X ( fnop ) X ( fnsave ) X ( fnstcw ) X ( fnstenv ) X ( fnstsw ) X ( fpatan ) X ( fprem ) X ( fprem1 ) X ( fptan ) X ( frndint ) X ( frstor ) \
	X ( fscale ) X ( fsin ) X ( fsincos ) X ( fsqrt ) X ( fst ) X ( fstp ) X ( fsub ) X ( fsubp ) X ( fsubr ) X ( fsubrp ) X ( ftst ) X ( fucom ) X ( fucomi ) X ( fucomip ) X ( fucomp ) X ( fucompp ) \
	X ( fxam ) X ( fxch ) X ( fxtract ) X ( fyl2x ) X ( fyl2xp1 ) \
	X ( addpd ) X ( addps ) X ( addsd ) X ( addss ) X ( addsubpd ) X ( addsubps ) X ( aesdec ) X ( aesdeclast ) X ( aesenc ) X ( aesenclast ) X ( aesimc ) X ( aeskeygenassist ) \
	X ( andnpd ) X ( andnps ) X ( andpd ) X ( andps ) X ( blendpd ) X ( blendps ) X ( blendvpd ) X ( blendvps ) \
	X ( cmppd ) X ( cmpps ) X ( cmpsd ) X ( cmpss ) X ( comisd ) X ( comiss ) \
	X ( cvtdq2pd ) X ( cvtdq2ps ) X ( cvtpd2dq ) X ( cvtpd2pi ) X ( cvtpd2ps ) X ( cvtpi2pd ) X ( cvtpi2ps ) X ( cvtps2dq ) X ( cvtps2pd ) X ( cvtps2pi ) \
	X ( cvtsd2si ) X ( cvtsd2ss ) X ( cvtsi2sd ) X ( cvtsi2ss ) X ( cvtss2sd ) X ( cvtss2si ) X ( cvttpd2dq ) X ( cvttpd2pi ) X ( cvttps2dq ) X ( cvttps2pi ) X ( cvttsd2si ) X ( cvttss2si ) \
	X ( divpd ) X ( divps ) X ( divsd ) X ( divss ) X ( dppd ) X ( dpps ) X ( extractps ) X ( haddpd ) X ( haddps ) X ( hsubpd ) X ( hsubps ) X ( insertps ) X ( lddqu ) \
	X ( maskmovdqu ) X ( maskmovq ) X ( maxpd ) X ( maxps ) X ( maxsd ) X ( maxss ) X ( minpd ) X ( minps ) X ( minsd ) X ( minss ) \
	X ( movapd ) X ( movaps ) X ( movd ) X ( movq ) X ( movddup ) X ( movdq2q ) X ( movdqa ) X ( movdqu ) X ( movhlps ) X ( movhpd ) X ( movhps ) X ( movlhps ) X ( movlpd ) X ( movlps ) \
	X ( movmskpd ) X ( movmskps ) X ( movntdq ) X ( movntdqa ) X ( movnti ) X ( movntpd ) X ( movntps ) X ( movntq ) X ( movq2dq ) X ( movsd ) X ( movshdup ) X ( movsldup ) X ( movss ) X ( movupd ) X ( movups ) \
	X ( mpsadbw ) X ( mulpd ) X ( mulps ) X ( mulsd ) X ( mulss ) X ( orpd ) X ( orps ) \
	X ( pabsb ) X ( pabsd ) X ( pabsw ) X ( packssdw ) X ( packsswb ) X ( packusdw ) X ( packuswb ) X ( paddb ) X ( paddd ) X ( paddq ) X ( paddsb ) X ( paddsw ) X ( paddusb ) X ( paddusw ) X ( paddw ) \
	X ( palignr ) X ( pand ) X ( pandn ) X ( pavgb ) X ( pavgw ) X ( pblendvb ) X ( pblendw ) X ( pclmulqdq ) X ( pcmpeqb ) X ( pcmpeqd ) X ( pcmpeqq ) X ( pcmpeqw ) \
	X ( pcmpestri ) X ( pcmpestrm ) X ( pcmpgtb ) X ( pcmpgtd ) X ( pcmpgtq ) X ( pcmpgtw ) X ( pcmpistri ) X ( pcmpistrm ) \
	X ( pextrb ) X ( pextrd ) X ( pextrq ) X ( pextrw ) X ( phaddd ) X ( phaddsw ) X ( phaddw ) X ( phminposuw ) X ( phsubd ) X ( phsubsw ) X ( phsubw ) \
	X ( pinsrb ) X ( pinsrd ) X ( pinsrq ) X ( pinsrw ) X ( pmaddubsw ) X ( pmaddwd ) X ( pmaxsb ) X ( pmaxsd ) X ( pmaxsw ) X ( pmaxub ) X ( pmaxud ) X ( pmaxuw ) \
	X ( pminsb ) X ( pminsd ) X ( pminsw ) X ( pminub ) X ( pminud ) X ( pminuw ) X ( pmovmskb ) \
	X ( pmovsxbd ) X ( pmovsxbq ) X ( pmovsxbw ) X ( pmovsxdq ) X ( pmovsxwd ) X ( pmovsxwq ) X ( pmovzxbd ) X ( pmovzxbq ) X ( pmovzxbw ) X ( pmovzxdq ) X ( pmovzxwd ) X ( pmovzxwq ) \
	X ( pmuldq ) X ( pmulhrsw ) X ( pmulhuw ) X ( pmulhw ) X ( pmulld ) X ( pmullw ) X ( pmuludq ) X ( por ) X ( psadbw ) X ( pshufb ) X ( pshufd ) X ( pshufhw ) X ( pshuflw ) X ( pshufw ) \
	X ( psignb ) X ( psignd ) X ( psignw ) X ( pslld ) X ( pslldq ) X ( psllq ) X ( psllw ) X ( psrad ) X ( psraw ) X ( psrld ) X ( psrldq ) X ( psrlq ) X ( psrlw ) \
	X ( psubb ) X ( psubd ) X ( psubq ) X ( psubsb ) X ( psubsw ) X ( psubusb ) X ( psubusw ) X ( psubw ) X ( ptest ) \
	X ( punpckhbw ) X ( punpckhdq ) X ( punpckhqdq ) X ( punpckhwd ) X ( punpcklbw ) X ( punpckldq ) X ( punpcklqdq ) X ( punpcklwd ) X ( pxor ) \
	X ( rcpps ) X ( rcpss ) X ( roundpd ) X ( roundps ) X ( roundsd ) X ( roundss ) X ( rsqrtps ) X ( rsqrtss ) X ( shufpd ) X ( shufps ) \
	X ( sqrtpd ) X ( sqrtps ) X ( sqrtsd ) X ( sqrtss ) X ( subpd ) X ( subps ) X ( subsd ) X ( subss ) X ( ucomisd ) X ( ucomiss ) \
	X ( unpckhpd ) X ( unpckhps ) X ( unpcklpd ) X ( unpcklps ) X ( xorpd ) X ( xorps ) \
	X ( vzeroupper ) X ( vzeroall ) X ( vbroadcastss ) X ( vbroadcastsd ) X ( vbroadcastf128 ) X ( vbroadcasti128 ) X ( vpbroadcastb ) X ( vpbroadcastw ) X ( vpbroadcastd ) X ( vpbroadcastq ) \
	X ( vpermilps ) X ( vpermilpd ) X ( vtestps ) X ( vtestpd ) X ( vcvtph2ps ) X ( vcvtps2ph ) X ( vpermps ) X ( vpermd ) X ( vpermq ) X ( vpermpd ) X ( vperm2f128 ) X ( vperm2i128 ) \
	X ( vinsertf128 ) X ( vextractf128 ) X ( vinserti128 ) X ( vextracti128 ) X ( vmaskmovps ) X ( vmaskmovpd ) X ( vpmaskmovd ) X ( vpmaskmovq ) \
	X ( vpsrlvd ) X ( vpsrlvq ) X ( vpsravd ) X ( vpsllvd ) X ( vpsllvq ) X ( vpblendd ) X ( vblendvps ) X ( vblendvpd ) X ( vpblendvb ) \
	X ( vfmaddsub132ps ) X ( vfmaddsub132pd ) X ( vfmsubadd132ps ) X ( vfmsubadd132pd ) X ( vfmadd132ps ) X ( vfmadd132pd ) X ( vfmadd132ss ) X ( vfmadd132sd ) \
	X ( vfmsub132ps ) X ( vfmsub132pd ) X ( vfmsub132ss ) X ( vfmsub132sd ) X ( vfnmadd132ps ) X ( vfnmadd132pd ) X ( vfnmadd132ss ) X ( vfnmadd132sd ) \
	X ( vfnmsub132ps ) X ( vfnmsub132pd ) X ( vfnmsub132ss ) X ( vfnmsub132sd ) \
	X ( vfmaddsub213ps ) X ( vfmaddsub213pd ) X ( vfmsubadd213ps ) X ( vfmsubadd213pd ) X ( vfmadd213ps ) X ( vfmadd213pd ) X ( vfmadd213ss ) X ( vfmadd213sd ) \
	X ( vfmsub213ps ) X ( vfmsub213pd ) X ( vfmsub213ss ) X ( vfmsub213sd ) X ( vfnmadd213ps ) X ( vfnmadd213pd ) X ( vfnmadd213ss ) X ( vfnmadd213sd ) \
	X ( vfnmsub213ps ) X ( vfnmsub213pd ) X ( vfnmsub213ss ) X ( vfnmsub213sd ) \
	X ( vfmaddsub231ps ) X ( vfmaddsub231pd ) X ( vfmsubadd231ps ) X ( vfmsubadd231pd ) X ( vfmadd231ps ) X ( vfmadd231pd ) X ( vfmadd231ss ) X ( vfmadd231sd ) \
	X ( vfmsub231ps ) X ( vfmsub231pd ) X ( vfmsub231ss ) X ( vfmsub231sd ) X ( vfnmadd231ps ) X ( vfnmadd231pd ) X ( vfnmadd231ss ) X ( vfnmadd231sd ) \
	X ( vfnmsub231ps ) X ( vfnmsub231pd ) X ( vfnmsub231ss ) X ( vfnmsub231sd ) \
	X ( vmovdqa32 ) X ( vmovdqa64 ) X ( vmovdqu8 ) X ( vmovdqu16 ) X ( vmovdqu32 ) X ( vmovdqu64 ) X ( vpandd ) X ( vpandq ) X ( vpandnd ) X ( vpandnq ) X ( vpord ) X ( vporq ) X ( vpxord ) X ( vpxorq ) \
	X ( vptestmb ) X ( vptestmw ) X ( vptestmd ) X ( vptestmq ) X ( vptestnmb ) X ( vptestnmw ) X ( vptestnmd ) X ( vptestnmq ) X ( vpternlogd ) X ( vpternlogq ) \
	X ( vpcmpb ) X ( vpcmpw ) X ( vpcmpd ) X ( vpcmpq ) X ( vpcmpub ) X ( vpcmpuw ) X ( vpcmpud ) X ( vpcmpuq ) X ( avx512 ) \
	X ( kmovb ) X ( kmovw ) X ( kmovd ) X ( kmovq ) X ( kortestb ) X ( kortestw ) X ( kortestd ) X ( kortestq ) X ( ktestb ) X ( ktestw ) X ( ktestd ) X ( ktestq ) \
	X ( kandb ) X ( kandw ) X ( kandd ) X ( kandq ) X ( kandnb ) X ( kandnw ) X ( kandnd ) X ( kandnq ) X ( knotb ) X ( knotw ) X ( knotd ) X ( knotq ) \
	X ( korb ) X ( korw ) X ( kord ) X ( korq ) X ( kxnorb ) X ( kxnorw ) X ( kxnord ) X ( kxnorq ) X ( kxorb ) X ( kxorw ) X ( kxord ) X ( kxorq ) \
	X ( kaddb ) X ( kaddw ) X ( kaddd ) X ( kaddq ) X ( kunpckbw ) X ( kunpckwd ) X ( kunpckdq )