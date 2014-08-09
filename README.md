gmpc-pauseafter-plugin
======================

Plugin for GMPC. Pauses playback after track/album ending or after N tracks played.


Installation
======================

I'm not familiar with all that './configure' stuff, so 
you (as I do) have to follow this very strange steps:

1. Download http://download.sarine.nl/Programs/gmpc/0.20.0/gmpc-playlistsort-0.20.0.tar.gz plugin.
2. Replace its 'plugin.c' file with the one from this git.
3. Do all './configure', 'make' and 'make install' as usual.
4. cd into directory, where 'make install' put your compiled playlistsortplugin.so and playlistsortplugin.la
(For me it was 'cd /usr/local/lib/gmpc/plugins/')
5. Perform 'mv playlistsortplugin.la pauseafter.la' and 'playlistsortplugin.so pauseafter.so'.


P.S.
=======================

Please, if some one can fix that 'Installation' and make it beautiful:
I would appreciate if you commit here rigth configuration, or Makefiles or stmh like that (autogen, mb?) 
