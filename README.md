# PlOS

![](https://encrypted-tbn0.gstatic.com/images?q=tbn:ANd9GcSOAsaricerkn1OjqMX8KsE4JZkMvvNm8eAfQ&usqp=CAU)

# About

- Higher half 32 bit kernel
- Beginner project


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
	- [ ] Aggiunte funzioni per manipolare le stringhe
- [x] Sviluppo pmm
	- [x] Inizializza pmm
	- [x] Inizializza tutte le regioni di spazio FREE
	- [x] Rimuovi la regione di spazio contenente il kernel
- [x] Implementa paging
	- [x] Implementa il vmm
	- [x] Sposta il kernel in higher half
- [ ] Implementa un heap
	- [ ] Implementa malloc() e free()
- [ ] Aggiungi supporto per la tastiera
- [ ] Implementa un file system

#### Seconda parte - User mode development
- [ ] Passare ad user mode
	- [ ] Aggiungere user mode entries nella gdt
	- [ ] Aggiungere un tss alla gdt
- [ ] Caricare un programma in memoria
	- [ ] Analizzare gli header dei file
	- [ ] Allocare memoria per essi
- [ ] Implementare le System Calls
- [ ] Aggiungere una toolchain
- [ ] Creare delle librerie C
- [ ] Creare Fork() ed Execute()
- [ ] Implementare una shell

#### Terza parte - Aggiungere funzioni al sistema operativo
- [ ] Creare una thread API
- [ ] Aggiungere la gestione del tempo 
- [ ] Implementare multithreading
- [ ] Sviluppo GUI
- [ ] Aggiungere networking
- [ ] Implementare il supporto per i suoni

# FIGA IL CERE