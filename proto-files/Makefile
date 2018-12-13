all: fileformat.pb.cc osmformat.pb.cc

fileformat.pb.cc:
	protoc --proto_path=. --cpp_out=. fileformat.proto

osmformat.pb.cc:
	protoc --proto_path=. --cpp_out=. osmformat.proto
