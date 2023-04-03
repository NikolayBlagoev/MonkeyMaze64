target_sources(FinalProject
	PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/generator/generator.cpp"

        "${CMAKE_CURRENT_LIST_DIR}/render/bloom.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/render/deferred.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/render/lighting.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/render/mesh.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/render/particle.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/render/scene.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/render/stb_image.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/render/texture.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/render/mesh_tree.cpp"

        "${CMAKE_CURRENT_LIST_DIR}/ui/camera.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/ui/menu.cpp")
