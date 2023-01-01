#pragma once
// �ȉ��� ifdef �u���b�N�́ADLL ����̃G�N�X�|�[�g��e�Ղɂ���}�N�����쐬���邽�߂�
// ��ʓI�ȕ��@�ł��B���� DLL ���̂��ׂẴt�@�C���́A�R�}���h ���C���Œ�`���ꂽ FILTERDLL1_EXPORTS
// �V���{�����g�p���ăR���p�C������܂��B���̃V���{���́A���� DLL ���g�p����v���W�F�N�g�ł͒�`�ł��܂���B
// �\�[�X�t�@�C�������̃t�@�C�����܂�ł��鑼�̃v���W�F�N�g�́A
// FILTERDLL1_API �֐��� DLL ����C���|�[�g���ꂽ�ƌ��Ȃ��̂ɑ΂��A���� DLL �́A���̃}�N���Œ�`���ꂽ
// �V���{�����G�N�X�|�[�g���ꂽ�ƌ��Ȃ��܂��B
#ifdef FILTERDLL1_EXPORTS
#define FILTERDLL1_API __declspec(dllexport)
#else
#define FILTERDLL1_API __declspec(dllimport)
#endif
extern "C"  __declspec(dllexport)
void markCharMap(char* inpath, char* outpath, int* ipara, int* outpara);
extern "C"  __declspec(dllexport)
void isHorV(char* inpath, char* outpath, int* ipara, int* outpara);
extern "C"  __declspec(dllexport)
void markChar(char* inpath, char* outpath, int* ipara, int* outpara);

extern "C"  __declspec(dllexport)
void dispTap(char* inpath, char* outpath, unsigned int* ipara, int* outpara);

extern "C"  __declspec(dllexport)
void LongSpaceChart(char* inpath, char* outpath1, char* outpath2, unsigned int* ipara, int* outpara);

extern "C"  __declspec(dllexport)
void XYVarChart(char* inpath, char* outpath1, char* outpath2, unsigned int* ipara, int* outpara);

extern "C"  __declspec(dllexport)
void XYGrayChart(char* inpath, char* outpath1, char* outpath2, unsigned int* ipara, int* outpara);

extern "C"  __declspec(dllexport)
void getPosColor(char* inpath, char* outpath, unsigned int* ipara, int* outpara);

extern "C"  __declspec(dllexport)
void circle(char* inpath, char* outpath, int* r, int* px, int* py, int* cr, int* cg, int* cb);

extern "C"  __declspec(dllexport)
void rectangle(char* inpath, char* outpath, int* inpara, int* outpara);

extern "C"  __declspec(dllexport)
void Poster3(char*, char*, int* inpara, int* outpara);

extern "C"  __declspec(dllexport)
void Poster2(char*, char*, unsigned int*, int*, int*);

extern "C"  __declspec(dllexport)
void Poster(char*, char*, unsigned int*, int*, int*);

extern "C"  __declspec(dllexport)
void C4(char*, char*, unsigned int*, int*, int*);

extern "C"  __declspec(dllexport)
void C3(char*, char*, int* inpara, int* outpara);

extern "C"  __declspec(dllexport)
void C2(char*, char*, unsigned int*, int*, int*);

extern "C"  __declspec(dllexport)
void C1(char*, char*, int*, int* );

extern "C"  __declspec(dllexport)
void makeHistgramList(char* inpath, int* hist);

extern "C"  __declspec(dllexport)
void makeHistgramChart(char* outpath, int* inpara, int* hist);

extern "C"  __declspec(dllexport)
void makeLocalHistgramList(char* inpath, int* ipara, int* hist);
////////////////////////////////////////////////////////////////////////
extern "C" __declspec(dllexport)
void VAR5(char* inpath, char* outpath, int* inpara, int* outpara);

extern "C" __declspec(dllexport)
void VAR4box(char* inpath, char* outpath, int* inpara, int* outpara);

extern "C" __declspec(dllexport)
void VAR4(char* inpath, char* outpath, int* inpara, int* outpara);

extern "C" __declspec(dllexport)
void VAR3(char* inpath, char* outpath, int* inpara, int* outpara);

extern "C" __declspec(dllexport)
void VAR2(char* inpath, char* outpath, int* inpara, int* outpara);

extern "C" __declspec(dllexport)
void VAR(char* inpath, char* outpath, int* inpara, int* outpara);

extern "C" __declspec(dllexport)
void BW(char* inpath, char* outpath, int* inpara, int* outpara);

extern "C" __declspec(dllexport)
void BW2(char* inpath, char* outpath, int* inpara, int* outpara);

extern "C" __declspec(dllexport)
void NON(char* inpath, char* outpath, int* inpara, int* outpara);

//extern "C" __declspec(dllexport)
//void getlen2(int*);
extern "C" __declspec(dllexport)
int  getlen();
//extern "C" __declspec(dllexport)
//void getname2(int*, char*, int*);
extern "C" __declspec(dllexport)
void getname(int n, char* fname, int* size);


std::string getPathDir(std::string outpath);
cv::Mat wimread(char* inpath);
void wimwrite(char* outpath, Mat img);

enum WRITING_DIR { Horizontal_writing, Vertical_writing };