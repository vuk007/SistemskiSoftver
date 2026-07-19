.section text
.equ a, 10 # definicija simbola a (apsolutni simbol)
ld $1, %r2  # konacna vrednost simbola a poznata u prvom prolazu
ld $1, %r2  # konacna vrednost simbola b poznata u drugom prolazu
.equ b, 20 # definicija simbola b (apsolutni simbol)
.extern c # deklaracija (uvoz) simbola c
ld $1, %r2 # konacnu vrednost simbola c ce znati tek linker
ld $1, %r2 # konacnu vrednost simbola d ce znati tek punilac
d: # definicija simbola d (relokativan simbol)
.word 10
.end
