#pragma once
#include "ListQueue.h"
#include <Windows.h>
#include <Shlwapi.h>

// Fuck the CPP .H and .CPP files 
// WTF!??!? Why duplicate the code? Crappy language.


class FileSystemEntity {
private:
	PSTR name;
	int size;

protected: 
	virtual void CalcSize() = 0;

	void SetSize(int size) {
		this->size = size;
	}

public:
	FileSystemEntity() {
		name = NULL;
		size = 0;
	}

	FileSystemEntity(PSTR name) {
		this->name = name;
		size = 0;
	}

	~FileSystemEntity() {
		if (name != NULL) {
			delete name;
		}
	}

	void SetName(PSTR name) {
		this->name = name;
	}

	PSTR GetName() {
		return name;
	}

	int GetSize() {
		if (size == 0) {
			CalcSize();
		}
		return size;
	}

	virtual void Delete() = 0;
};

class File : public FileSystemEntity {
protected:
	void CalcSize() {
		HANDLE hFile = CreateFile (GetName(), 0, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
		DWORD size;

		if (hFile != INVALID_HANDLE_VALUE) {
			GetFileSize (hFile, &size);
			CloseHandle (hFile);
		}

		SetSize(size);
	}

public: 
	File(PSTR name) : FileSystemEntity(name) {
	}

	void Delete() {
		DeleteFile(GetName());
	}
};



class EntityNode: public ListNode {
private:
	FileSystemEntity* fsEntity;

public:
	EntityNode(FileSystemEntity* entity) : ListNode() {
		fsEntity = entity;
	}

	~EntityNode() {
		delete fsEntity;
	}

	FileSystemEntity* GetEntity() {
		return fsEntity;
	}

	void SetEntity(FileSystemEntity* entity) {
		this->fsEntity = entity;
	}
};

class Directory : public FileSystemEntity {
private:
	ListQueue* innerEntities;

protected:
	void CalcSize() {
		int size = 0;
		for(int iter = 0; iter < innerEntities->GetCount(); iter++) {
			size += ((EntityNode*)innerEntities->GetNext())->GetEntity()->GetSize();
		}

		SetSize(size);
	}

public:
	Directory(PSTR name) : FileSystemEntity(name) {
		innerEntities = new ListQueue();
	}

	~Directory() {
	}

	void AddEntity(FileSystemEntity* entity) {
		innerEntities->Enqueue(new EntityNode(entity));
	}

	void Delete() {
		for(int iter = 0; iter < innerEntities->GetCount(); iter++) {
			((EntityNode*)innerEntities->GetNext())->GetEntity()->Delete();
		}

		RemoveDirectory(GetName());
	}

	void DeleteOldestEntity() {
		EntityNode *prevOldest = NULL, *oldest = ((EntityNode*)innerEntities->GetNext()); // first entity
		EntityNode *prevNode = NULL, *next = NULL;

		for(int iter = 1; iter < innerEntities->GetCount(); iter++) {
			prevNode = next;
			next = ((EntityNode*)innerEntities->GetNext());
			if (strcmp(oldest->GetEntity()->GetName(), next->GetEntity()->GetName()) < 0) {
				oldest = next;
				prevOldest = prevNode;
			}
		}

		oldest->GetEntity()->Delete(); // Delete file/directory
		innerEntities->Remove(prevOldest); 
	}

	FileSystemEntity* FindEntity(PSTR name) {
		int entitiesCount = innerEntities->GetCount();
		EntityNode* current;

		for (int i= 0; i<entitiesCount; i++) {
			current = (EntityNode*)innerEntities->GetNext();

			if (strcmp(current->GetEntity()->GetName(), name) == 0) {
				return current->GetEntity();
			}
		}

		return NULL;
	}
};
