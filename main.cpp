#include "Build_Options.h"

FlashMem* flashmem = NULL;

MAPPING_METHOD mapping_method = MAPPING_METHOD::NONE; //���� �÷��� �޸��� ���ι�� (default : ������� ����)
TABLE_TYPE table_type = TABLE_TYPE::STATIC; //���� ���̺��� Ÿ��

void main() 
{
	flashmem->bootloader(flashmem, mapping_method, table_type);

	while (1)
	{
		flashmem->disp_command(mapping_method, table_type); //Ŀ�ǵ� ���
		flashmem->input_command(flashmem, mapping_method, table_type); //Ŀ�ǵ� �Է�
	}
}