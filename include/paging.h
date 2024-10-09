#pragma once
#include <stddef.h>
#include <stdint.h>

#include <stdint.h>

#define PAGE_PRESENT 0x1 // Présent
#define PAGE_RW 0x2		 // Lecture/Écriture
#define PAGE_USER 0x4	 // Accès utilisateur
#define PAGE_SIZE 4096	 // Taille d'une page en octets

#define PAGE_DIRECTORY_SIZE 1024
#define PAGE_TABLE_SIZE 1024

// Structure pour la Page Directory et la Page Table
extern uint32_t page_directory[PAGE_DIRECTORY_SIZE];
extern uint32_t page_table[PAGE_TABLE_SIZE];
// Fonction d'initialisation du paging avec assembleur inline

void init_paging();
