#include "gamewindow.h"
#include "layermanager.h"

namespace wsp{
	LayerManager::LayerManager(u32 boundary) :
		_layers(NULL), _size(0), _boundary(boundary)
	{
		// Initialize our layers for usage
		_layers = new Layer*[_boundary];
		for(u32 i = 0; i < _boundary; i++)_layers[i] = NULL;
	}

	LayerManager::~LayerManager(){
		if(_layers)
			delete[] _layers; _layers = NULL;
	}

	void LayerManager::Append(Layer* layer){
		if(layer == NULL)return;
		// Remove it first from the Manager
		Remove(layer);
		// Check if it can be appended
		if(_size >= _boundary)return;

		// Set layer on farthest position
		_layers[_size] = layer;
		_size++;
	}

	void LayerManager::Insert(Layer* layer, u32 index){
		if(layer == NULL || index > _size)return;
		// Remove it first from the Manager
		Remove(layer);
		// Check if it can be inserted
		if(_size >= _boundary || index > _size)return;

		// Make some space for our new layer
		for(u32 i = _size; i > index; i--){
			_layers[i] = _layers[i-1];
		}
		_layers[index] = layer;
		_size++;
	}

	int LayerManager::GetIndex(Layer* layer){
		if(layer == NULL)return -1;

		for(u32 i = 0; i < _size; i++){
			if(layer == _layers[i]){
				return i;
			}
		}
        return -1;
	}

	void LayerManager::Remove(Layer* layer){
		if(layer == NULL)return;

		bool shift = false;

		for(u32 i = 0; i < _size; i++){
			// Or check if the layers are equal and turn shifting on
			if(layer == _layers[i]){
				_layers[i] = NULL;
				shift = true;
			}
			// Layer found and shifting everything forward
			if(shift) {
                if( i+1 < _size){
				    _layers[i] = _layers[i+1];
                }else{
                    _layers[i] = NULL;
                }
			}
		}

		if(shift)_size--; // Success!
	}

	void LayerManager::RemoveAll(){
		for(u32 i = 0; i < _size; i++){
			_layers[i] = NULL;
		}
		_size = 0;
	}

	// TODO: Maybe add sorting?

	Layer* LayerManager::GetLayerAt(u32 index) const{
		if(index >= _size)return NULL;
		return _layers[index];
	}

	u32 LayerManager::GetSize() const{
		return _size;
	}

	void LayerManager::Draw(void) const{
		// Do the layers!
		for(u32 i = _size; i > 0; i--){
			if(i == 0) break;
			if(_layers[i-1])
				_layers[i-1]->Draw(0, 0);
		}
	}
};
