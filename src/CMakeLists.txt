target_sources(FinalProject
	PRIVATE
        # "${CMAKE_CURRENT_LIST_DIR}/networking/client.cpp" # Uncomment when a cross-platform solution is working

		"${CMAKE_CURRENT_LIST_DIR}/render/mesh.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/render/scene.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/render/stb_image.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/render/texture.cpp"
        
        "${CMAKE_CURRENT_LIST_DIR}/ui/camera.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/ui/menu.cpp")
