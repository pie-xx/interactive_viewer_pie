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
void circle(char* inpath, char* outpath, int* r, int* px, int* py, int* cr, int* cg, int* cb);

extern "C"  __declspec(dllexport)
void Poster3(char*, char*, unsigned int*, int*, int*);

extern "C"  __declspec(dllexport)
void Poster2(char*, char*, unsigned int*, int*, int*);

extern "C"  __declspec(dllexport)
void Poster(char*, char*, unsigned int*, int*, int*);

extern "C"  __declspec(dllexport)
void C4(char*, char*, unsigned int*, int*, int*);

extern "C"  __declspec(dllexport)
void C3(char*, char*, unsigned int*, int*, int*);

extern "C"  __declspec(dllexport)
void C2(char*, char*, unsigned int*, int*, int*);

extern "C"  __declspec(dllexport)
void C1(char*, char*, unsigned int*, int*, int*);

extern "C" __declspec(dllexport)
void BW(char*, char*, unsigned int*, int*, int*);

extern "C" __declspec(dllexport)
void NON(char*, char*, unsigned int*, int*, int*);

extern "C" __declspec(dllexport)
void getlen(int*);

extern "C" __declspec(dllexport)
void getname(int*, char*, int*);
