// GL ERROR CHECK
#define CHECK_GL_ERROR() CheckGLError(__FILE__, __LINE__)
#include <iostream>
// GL ERROR CHECK
int CheckGLError(char *file, int line)
{
	//return 0;
	GLenum glErr,glErr2;
	int retCode = 0;

	glErr = glErr2 = glGetError();
	while (glErr != GL_NO_ERROR) 
	{
	   char* str1 = (char*)gluErrorString(glErr);
	   if (str1)
			cout << "GL Error #" << glErr << "(" << str1 << ") " << " in File " << file << " at line: " << line << endl;
	   else
			cout << "GL Error #" << glErr << " in File " << file << " at line: " << line << endl;
		retCode = 1;
		glErr = glGetError();
	}
	if (glErr2 != GL_NO_ERROR) while(1)Sleep(100);;

	return 0;
}
///////////////////////////////////////////
class Shader
{
public:
	Shader(std::string shadername){name=shadername;};
	void attach(int type,char* filename)
	{
		char* mem=read_file(filename);
		if(mem==0)
		{
			printf("Shader file %s not found\n",filename);
			while(1);;
		}
		GLuint handle = glCreateShader(type);CHECK_GL_ERROR();
		glShaderSource(handle, 1, (const GLchar**)(&mem), 0);CHECK_GL_ERROR();
		glCompileShader(handle);CHECK_GL_ERROR();

		GLint compileSuccess=0;
		GLchar compilerSpew[256];

		glGetShaderiv(handle, GL_COMPILE_STATUS, &compileSuccess);
		CHECK_GL_ERROR();
		if(!compileSuccess)
		{
			glGetShaderInfoLog(handle, sizeof(compilerSpew), 0, compilerSpew);
			printf("Shader %s\n%s\ncompileSuccess=%d\n",filename,compilerSpew,compileSuccess);
			CHECK_GL_ERROR();
			while(1);;
		}
		free(mem);
		handles.push_back(handle);
	}
	void link()
	{
		program_handle = glCreateProgram();
		for (int i=0;i<handles.size();i++)
		{
			glAttachShader(program_handle, handles[i]);
			CHECK_GL_ERROR();
		}

		glLinkProgram(program_handle);
		CHECK_GL_ERROR();

		GLint linkSuccess;
		GLchar compilerSpew[256];
		glGetProgramiv(program_handle, GL_LINK_STATUS, &linkSuccess);
		if(!linkSuccess)
		{
			glGetProgramInfoLog(program_handle, sizeof(compilerSpew), 0, compilerSpew);
			printf("Shader Linker:\n%s\nlinkSuccess=%d\n",compilerSpew,linkSuccess);
			CHECK_GL_ERROR();
			while(1);;
		}
		printf("%s linked successful\n",name.c_str());
		CHECK_GL_ERROR();
	}
	void setUniformMatrix4fv(char* varname, GLsizei count, GLboolean transpose, GLfloat *value)
	{
		GLint loc = glGetUniformLocation(program_handle,varname);
		if (loc==-1) 
		{
			printf("Variable \"%s\" in shader \"%s\" not found\n",varname,name.c_str());
			while(1);;
		};
		glUniformMatrix4fv(loc, count, transpose, value);
		CHECK_GL_ERROR();
	}
	void setUniform4f(char* varname, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3)
	{ 
		GLint loc = glGetUniformLocation(program_handle,varname);
		if (loc==-1) 
		{
			printf("Variable \"%s\" in shader \"%s\" not found\n",varname,name.c_str());
			while(1);;
		};
		glUniform4f(loc, v0, v1, v2, v3);
		CHECK_GL_ERROR();
	}
	void setUniform1i(char* varname, GLint v0)
	{ 
		GLint loc = glGetUniformLocation(program_handle,varname);
		if (loc==-1) 
		{
			printf("Variable \"%s\" in shader \"%s\" not found\n",varname,name.c_str());
			//while(1);;
		};
		glUniform1i(loc, v0);
		CHECK_GL_ERROR();
	}
	void begin(void)
	{
		glUseProgram(program_handle);
		CHECK_GL_ERROR();
	}
	void end(void)
	{
		glUseProgram(0);
		CHECK_GL_ERROR();
	}
private:
	std::vector<GLuint> handles;
	GLuint program_handle;
	std::string name;

	char* read_file(char* name)
	{
		FILE * fp = fopen (name, "rb");
		
		if (fp==0) 
		{
			printf ("File %s NOT FOUND\n");
			while(1);;		
		}
		fseek(fp, 0L, 2);
		int fsize = ftell(fp);
		fseek(fp, 0L, 0);
		char* mem=(char*)malloc(fsize+1);
		for(int i=0;i<fsize+1;i++)mem[i]=0;
		fread (mem, 1, fsize, fp);
		fclose (fp);
		return mem;
	}
};
