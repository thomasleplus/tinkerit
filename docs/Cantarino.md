#summary Cantarino, the Arduino speech synthesiser
#labels Featured
http://farm4.static.flickr.com/3437/3717469268_4234104752_m.jpg

= Introduction =

Cantarino is a software speech synthesiser for Arduino. It is still a work in progress, but a tech demo that runs on Arduino is now available.

= Demo =

==[http://vimeo.com/5577046 Demo video]==

Demo presented at Music Hack Day is [http://tinkerit.googlecode.com/files/speech_daisybell.pde downloadable here]. Audio out is on Arduino pin 3. A schematic is at the end of the video.

= Community =

This is a work in progress. Sign up to [http://groups.google.com/group/cantarino the Google Group] to watch or guide Cantarino's development.

= Technical: Formant synthesiser =

An interrupt on the PWM output timer calls a sample generator. This uses a synthesis similar to FOF or granule synthesis - using two sines and a square wave of adjustable pitch and amplitude, synced and enveloped by a pitch of selectable frequency. Phase modulation is added to the pitch, to allow unvoiced phonemes to be uttered.

= Phoneme codebook =

The following phonemes are currently supported, based on data from the SAM Apple ][ synthesiser.

|| IY || f *ee* t ||
|| IH || p *i* n ||
|| EH || b *e* g ||
|| AE || S *a* m ||
|| AA || p *o* t ||
|| AH || t *a* lk ||
|| AO || c *o* ne ||
|| UH || b *oo* k ||
|| UX || l *oo* t ||
|| ER || b *i* rd ||
|| AX || g *a* llon ||
|| IX || d *i* git ||
|| EY || m *a* de (dipthong start) ||
|| AY || h *igh* (dipthong start) ||
|| OY || b *oy* (dipthong start) ||
|| AW || h *ow* (dipthong start)||
|| OW || sl *ow* (dipthong start)||
|| UW || cr *ew* (dipthong start) ||
|| YX || h *igh* (dipthong end) ||
|| WX || h *ow* (dipthong end) ||
|| RX || R after vowel ||
|| LX || L after vowel ||
|| _X || H before consonant ||
|| DX || flap ||
|| R || *r* ed ||
|| L || a *ll* ow ||
|| W || a *w* ay ||
|| WH || *wh* ale ||
|| Y || *y* ou ||
|| M || *m* usic hack day ||
|| N || ma *n* ||
|| NX || so *ng* ||
|| B || *b* ad ||
|| D || *d* og ||
|| G || *g* uardian offices ||
|| J || *j* u *dg* e ||
|| Z || *z* oo ||
|| ZH || plea *s* ure ||
|| V || se *v* en ||
|| DH || *th* en ||
|| S || *S* unday 12th July 2009 ||
|| SH || fi *sh* ||
|| F || *f* ish ||
|| TH || *th* in ||
|| P || *p* oke ||
|| T || *t* inker.it ||
|| K || ca *k* e ||
|| CH || spee *ch* ||
|| _H || a *h* ead ||
|| Q || glottal stop ||

= Credits =
Formant synthesis system: inspired by SAM
Logo: cropped from http://www.flickr.com/photos/jeanbaptisteparis/119421176/