#include "FlashMem.h"

FlashMem* flashmem = NULL;

int mapping_method = 0; //���� �÷��� �޸��� ���ι�� (default : 0 - ������� ����)
int table_type = 0; //���� ���̺��� Ÿ�� (0 : Static Table, 1 : Dynamic Table)

void main() {
	flashmem->bootloader(&flashmem, mapping_method, table_type);

	while (1)
	{
		flashmem->disp_command(mapping_method, table_type); //Ŀ�ǵ� ���
		flashmem->input_command(&flashmem, mapping_method, table_type); //Ŀ�ǵ� �Է�
	}
}