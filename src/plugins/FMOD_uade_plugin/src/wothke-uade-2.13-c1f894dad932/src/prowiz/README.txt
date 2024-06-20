This is based on ProWizard (http://asle.free.fr/prowiz/) version 1.71 that Sylvain "Asle" Chipaux
had kindly supplied to me.

The primary purpose of the original tool seems to be the ripping of sound data from some
larger surrounding binary. As an add-on it then allows to convert *some* of these formats
to Protracker.

The format detection logic has a certain functional overlap to UADE's impl in amifilemagic.c 
and it probably does a better job in some scenarios. (And it would be beneficial to get rid
of the file extension/prefix mapping garbage that UADE is using - and to exclusively rely on
a file content based detection instead.) However Prowiz' impl is limited to tracker formats
and it doesn't handle certain Amiga formats at all - incorrectly misdiagnosing those as
something else. For now I don't feel like merging two fragile "heuristics based" detection 
implementations.. instead I'll just use the "convert to Protracker" parts - but only
where it covers an area that isn't handled by UADE' old implementation already (see
stripped down prowiz.c).



With regard to mod-files there are various problems with UADE's original implementation:

- It seems that a multitude of different "packers" have been used on the Amiga - 
  obfuscating whatever "tracker" some music file might originally have been created in.
  Only some of these are known to UADE's impl, meaning that the others cannot be played
  at all.

- There are also many variants/versions of used "trackers" - and the respective file formats
  differ in more or less significant ways. In order to play them the correct player version
  must be identified. Infortunately UADE's impl here is severely flawed: Its PTK-Prowiz
  EaglePlayer does not know all the formats. And then the player version detection is often
  based only on the file name extension - often causing some incorrect player version to be used.  

  
This Prowiz impl here knows how to convert some more formats to Protracker. By using its 
conversion during UADE' file loading, UADE now is able to play these additional formats.

Trade-offs? It is unclear how lossy the used conversions might actually be: The differences
in original tracker versions were usually due to added playback features that had not been 
available in earlier versions. It might be that the generic Protracker format used as a
target is able to handle the subtilities of all these features 100% accurately. But even
if it did it is likely that the conversion logic might mess up the respective translation
once in a while. But on the other hand it is unclear how faithfully UADE's existing PTK-Prowiz
player actually played the various formats and and it might already be using lossy
conversions of its own.. (the PTK-Prowiz player did something similar after all). For now 
I am keeping UADE' old impl as long as I am not aware of any blatant issues. (I am keeping 
all the unused Prowiz code in the "r" sub-folder. It can potentially be re-activated in case 
that need should arise in the future.)
