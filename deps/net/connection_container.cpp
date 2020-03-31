#include "connection_container.h"
#include <assert.h>
#include <cstdio>

inline bool ConnectionContainer::isLegal(int key) {
	if (key >= 0 && key < capacity) {
		return true;
	}
	return false;
}

ConnectionContainer::ConnectionContainer(int cap) {
	assert(cap > 0);
	capacity = cap;
	size = 0;
    valueArray = new Value[capacity + 1];
	assert(valueArray != nullptr);
}

ConnectionContainer::~ConnectionContainer() {
	assert(valueArray != nullptr);
	delete[] valueArray;
}

int ConnectionContainer::Size(){
    return size;
}

Connection* ConnectionContainer::Get(int key) {
	if (isLegal(key) && valueArray[key].use) {
		return valueArray[key].conn;
	}
	return nullptr;
}

bool ConnectionContainer::Set(int key, Connection* c) {
	if (!isLegal(key)) {
		return false;
	}
	if (!valueArray[key].use) {
		++size;
	}
    valueArray[key].use = true;
	valueArray[key].conn = c;
	return true;
}

bool ConnectionContainer::Remove(int key) {
	if (!isLegal(key)) {
		return true;
	}
	if (valueArray[key].use) {
		--size;
	}
	valueArray[key].use = false;
    valueArray[key].conn = nullptr;
	return true;
}
