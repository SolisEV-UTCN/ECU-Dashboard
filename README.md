# AUSTRALIAN WORLD SOLAR CHALLANGE 2025 FINAL STATE CODE.
## CODUL PREZINTA MODIFICARI PENTRU BYPASS-UL ANUMITOR BUTOANE CE AU AVUT MICI PROBLEME IN TIMPUL COMPETIȚIEI.
## TRANSMITEREA MESAJELOR CATRE BMS E DISABLED PENTRU CA BATERIA A FUNCTIONAT IN STANDALONE.
		
# ADRESE BUTOANE VOLAN:

A0 in dreapta de tot, A1 mijloc, A2 stanga

	000 - DISPLAY_SWITCH D0
	001 - SIGN_RIGHT     D1
	010 - SIGN_LEFT      D2
	011 - BRAKE SWAP     D3
	100 - CRUISE_DOWN    D4
	101 - CRUISE_UP      D5
	110 - CRUISE ON      D6
	111 - AVARIE         D7 
	
# idei care nu mergeau:
## Codul De Transmitere telemetrie catre MPPT nu functioneaza cum trebuie.
## Atentie la timeout-urile de la comunicatii produse de task-uri prea indelungate..
		
