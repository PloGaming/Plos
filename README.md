# PLOS

![](https://encrypted-tbn0.gstatic.com/images?q=tbn:ANd9GcSOAsaricerkn1OjqMX8KsE4JZkMvvNm8eAfQ&usqp=CAU)

# About

- Higher half 32 bit kernel
- Beginner project

# Screenshot

![alt text](https://cdn.discordapp.com/attachments/994319172530098216/1019287258077069393/Screenshot_21.png)

# Progresso

#### Prima parte - kernel development
- [x] GRUB multiboot
- [x] Setup iniziale
- [x] Setup gdt
- [x] A20 line abilitata
- [x] Text mode implementata
    - [x] Aggiunto lo scrolling e new line nello schermo
- [x] Interrupt aggiunti
	- [x] Crea ed inizializza la IDT
	- [x] Inizializza gli ISR
	- [x] Implementa gli IRQ
		- [x] Sviluppo driver per PIC
- [x] Sviluppo funzioni libc
	- [x] Aggiunta la funzione printf()
	- [x] Aggiunte memset() e memcpy()
	- [x] Aggiunte funzioni per manipolare le stringhe
- [x] Sviluppo pmm
	- [x] Inizializza pmm
	- [x] Inizializza tutte le regioni di spazio FREE
	- [x] Rimuovi la regione di spazio contenente il kernel
- [x] Implementa paging
	- [x] Implementa il vmm
	- [x] Implementa recursive paging
	- [x] Sposta il kernel in higher half
- [x] Implementa un heap
	- [x] Implementa malloc() 
	- [x] Implementa free() 
- [x] Aggiungi supporto per la tastiera
- [x] Aggiungi una pseudo-shell
- [x] Implementa un timer (PIT)
- [ ] Implementa un file system

#### Seconda parte - User mode development
- [ ] Passare ad user mode
	- [ ] Aggiungere user mode entries nella gdt
	- [ ] Aggiungere un tss alla gdt
- [ ] Caricare un programma in memoria
	- [ ] Analizzare gli header dei file
	- [ ] Allocare memoria per essi
-......
