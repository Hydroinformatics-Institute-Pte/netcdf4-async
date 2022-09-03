#ifndef NETCDF_FILE_H
#define NETCDF_FILE_H

#include <string>

class NetCDFFile {
    private:
        int id;
	    std::string name;
	    int file_format;
	    bool closed;
        void open(const char *filename, std::string mode, int &format);

    public:
	    explicit NetCDFFile(std::string name, std::string mode, std::string format);
        explicit NetCDFFile(int& id);
	    ~NetCDFFile();
	    	
	    void Close();
    	void Sync();
	    void DataMode();
	    int GetId();
	    std::string GetName();
	    bool IsClosed();
	    std::string GetFormat();
	    std::string Inspect();
};

#endif