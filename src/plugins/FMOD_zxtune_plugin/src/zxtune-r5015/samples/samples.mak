samples_path := $(dirs.root)/samples/chiptunes

samples_ay38910 := \
	as0/Samba.as0 \
	asc/SANDRA.asc \
	ay/AYMD39.ay \
	ayc/MEGAPAB2.AYC \
	ftc/Nostalgy.ftc \
	gtr/L.Boy.gtr \
	psc/FL_SH_EI.psc \
	psg/Illusion.psg \
	psm/Calamba.psm \
	pt1/GoldenGift.pt1 \
	pt2/PITON.pt2 \
	pt3/Speccy2.pt3 \
	sqt/taiobyte.sqt \
	st1/EPILOG.st1 \
	st3/Kvs_Joke.st3 \
	stc/stracker.stc \
	stp/iris_setup.stp \
	ts/INEEDREST.ts \
	txt/C_IMPROV.txt \
	vtx/Enlight3.vtx \
	ym/Kurztech.ym

samples_ay38910_dir := AY-3-8910

samples_dac_zx := \
	chi/ducktale.chi \
	dmm/popcorn.dmm \
	dst/Untitled.dst \
	et1/mortal.m \
	pdt/TechCent.M \
	sqd/Maskarad.sqd \
	str/COMETDAY.str

samples_dac_zx_dir := DAC/ZX

samples_dac_amiga := \
  ahx/Stormlord.ahx
  
samples_dac_amiga_dir := DAC/Amiga

samples_dac_sega := \
  dsf/m20.dsf

samples_dac_sega_dir := DAC/Sega

samples_mos6581 := \
        sid/Love_Is_a_Shield.sid

samples_mos6581_dir := MOS6581

samples_saa1099 := \
	cop/axel.cop

samples_saa1099_dir := SAA1099

samples_ym2203 := \
	tf0/ducktales1_moon.tf0 \
	tfc/unbeliev.tfc \
	tfd/mass_production.tfd \
	tfe/disco.tfe

samples_ym2203_dir := YM2203

samples_spc700 := \
	spc/ala-16.spc
   
samples_spc700_dir := SPC700

samples_multi := \
	gym/TheLionKing.gym \
	kss/MSX_Fan.kss \
	mtc/CyberMotion.mtc \
	vgm/EarthwormJim.vgm
   
samples_multi_dir := Multi

samples_rp2a0x := \
	nsf/knifus.nsf \
	nsfe/SuperContra.nsfe
   
samples_rp2a0x_dir := RP2A0X

samples_lr35902 := \
	gbs/stripped.gbs

samples_lr35902_dir := LR35902

samples_co12294 := \
	sap/intro.sap
   
samples_co12294_dir := CO12294

samples_huc6270 := \
	hes/raiden.hes
   
samples_huc6270_dir := HuC6270

define install_samples_cmd
	$(call makedir_cmd,$(DESTDIR)/Samples/$(samples_$(1)_dir))
	$(foreach smp,$(samples_$(1)),$(call copyfile_cmd,$(samples_path)/$(samples_$(1)_dir)/$(smp),$(DESTDIR)/Samples/$(samples_$(1)_dir)) && ) \
	echo Installed $(1) samples
endef

install_samples_dac: install_samples_dac_zx install_samples_dac_amiga install_samples_dac_sega

install_samples: install_samples_ay38910 install_samples_dac install_samples_mos6581 install_samples_saa1099 install_samples_ym2203 install_samples_spc700 \
  install_samples_multi install_samples_rp2a0x install_samples_lr35902 install_samples_co12294 install_samples_huc6270

install_samples_ay38910:
	$(call install_samples_cmd,ay38910)

install_samples_dac_zx:
	$(call install_samples_cmd,dac_zx)

install_samples_dac_amiga:
	$(call install_samples_cmd,dac_amiga)

install_samples_dac_sega:
	$(call install_samples_cmd,dac_sega)

install_samples_mos6581:
	$(call install_samples_cmd,mos6581)

install_samples_saa1099:
	$(call install_samples_cmd,saa1099)

install_samples_ym2203:
	$(call install_samples_cmd,ym2203)

install_samples_spc700:
	$(call install_samples_cmd,spc700)

install_samples_multi:
	$(call install_samples_cmd,multi)
  
install_samples_rp2a0x:
	$(call install_samples_cmd,rp2a0x)

install_samples_lr35902:
	$(call install_samples_cmd,lr35902)

install_samples_co12294:
	$(call install_samples_cmd,co12294)

install_samples_huc6270:
	$(call install_samples_cmd,huc6270)

install_samples_playlist: 
	$(call copyfile_cmd,$(dirs.root)/samples/samples.xspf,$(DESTDIR))
