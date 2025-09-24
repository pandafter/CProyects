#include <unistd.h>
#include <stdarg.h>
#include <string.h>
#include <stdbool.h>



char* int_to_string(int n) {
    static char buffer[20];
    int i = 0;
    
    if (n < 0) {
        buffer[i++] = '-';
        n = -n;
    }
    
    if (n == 0) {
        buffer[i++] = '0';
        buffer[i] = '\0';
        return buffer;
    }
    
    int digit_count = 0;
    int temp = n;
    
    while (temp > 0) {
        digit_count++;
        temp /= 10;
    }
    
    int pos = i + digit_count - 1;
    while (n > 0) {
        buffer[pos--] = n % 10 + '0';
        n /= 10;
    }
    
    buffer[i + digit_count] = '\0';
    return buffer;
}

char* float_to_string(float n, int decimals) {
    static char buffer[30];
    int pos = 0;
    
    if (n < 0) {
        buffer[pos++] = '-';
        n = -n;
    }
    
    int int_part = (int)n;
    float decimal_part = n - int_part;
    
    char* int_str = int_to_string(int_part);
    int int_len = strlen(int_str);
    
    for (int i = 0; i < int_len; i++) {
        buffer[pos++] = int_str[i];
    }
    

    buffer[pos++] = '.';
    

    if (decimals > 0) {

        int multiplier = 1;
        for (int i = 0; i < decimals; i++) {
            multiplier *= 10;
        }
        
        int decimal_int = (int)(decimal_part * multiplier);
        
        char* decimal_str = int_to_string(decimal_int);
        int decimal_len = strlen(decimal_str);
        
        for (int i = decimal_len; i < decimals; i++) {
            buffer[pos++] = '0';
        }
        
        for (int i = 0; i < decimal_len; i++) {
            buffer[pos++] = decimal_str[i];
        }
    }

    buffer[pos] = '\0';
    
    return buffer;
}

char* bool_to_string(int n) {
	static char* false_str = "false";
	static char* true_str = "true";

	return (n == 0) ? false_str : true_str;
}


char* int_to_hex_string(int n) {
	static char buffer[20];
	int i = 0;


	if (n == 0) {
		buffer[i++] = '0';
		buffer[i] = '\0';
		return buffer;
	}

	while (n > 0) {
		int digit = n % 16;
		if (digit < 10) {
			buffer[i++] = digit + '0';
		} else {
			buffer[i++] = digit - 10 + 'a';
		}
		n /= 16;
	}


    int start = 0;
    int end = i - 1;
    while (start < end) {
        char temp = buffer[start];
        buffer[start] = buffer[end];
        buffer[end] = temp;
        start++;
        end--;
    }


    for (int j = i - 1; j >= 0; j--) {
        buffer[j + 2] = buffer[j];
    }
    buffer[0] = '0';
    buffer[1] = 'x';
    
    buffer[i + 2] = '\0';
    return buffer;
}


void my_printf(const char *format, ...) {
	va_list args;
	va_start(args, format);


	for (int i = 0; format[i] != '\0'; i++)
	{
		if (format[i] == '%') {
			i++;
			switch (format[i])
			{
				case 's' : { char *s = va_arg(args, char*); write(1, s, strlen(s)); break;}
				case 'd' : { int n = va_arg(args, int); char* result = int_to_string(n); write(1, result, strlen(result)); break;}
				case 'c' : { char c = va_arg(args, int); write(1, &c, 1); break;}
				case 'f' : { float f = va_arg(args, double); write(1, float_to_string(f, 2), strlen(float_to_string(f, 2))); break;}
				case 'b' : { int b = va_arg(args, int); write(1, bool_to_string(b), strlen(bool_to_string(b))); break;}
				case 'x' : { int n = va_arg(args, int); char* result = int_to_hex_string(n); write(1, result, strlen(result)); break;}
			}
		} else {
			write(1, &format[i], 1);
		}
	}
	

	va_end(args);
}



int main() {


	char* nombre = "Juan";

	char* pais = "Colombia";

	int a = 123;
	int b = 245;

	int resultado = 15456;

	float c = 1.23123;

	bool miBool =  a < b;



	my_printf("Hola, %s!, de %s\n la suma de %d y %d es: %d\n que en hexadecimal es: %x\n identificado con tu caracter: %c\n con tu float: %f\n tu bool es: %b\n" , nombre, pais, a, b, resultado, resultado, nombre[0], c, miBool);
	return 0;
}
