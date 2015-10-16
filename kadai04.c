#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>


//=====================================================================
//ɬ�פʥǡ���
#define FILENAME "image.ppm"
#define MAGICNUM "P3"
#define WIDTH 256
#define WIDTH_STRING "256"
#define HEIGHT 256
#define HEIGHT_STRING "256"
#define MAX 255
#define MAX_STRING "255"
#define FOCUS 256.0
#define Z_BUF_MAX 

//diffuseColor���Ǽ��������
double diffuse_color[3];
//shiness���Ǽ�����ѿ�
double shininess;
//specularColor���Ǽ�����ѿ�
double specular_color[3];

//������ǥ��ʿ�Ը���

//��������
const double light_dir[3] = {-1.0, -1.0, 2.0};
//�������뤵
const double light_rgb[3] = {1.0, 1.0, 1.0};

//�������֤ϸ����Ǥ����ΤȤ�����Ƥ�Ԥ�.

//=====================================================================


//������˲����������ΰ�����
double image[HEIGHT][WIDTH][3];
//z�Хåե��Ѥ��ΰ�����
double z_buf[HEIGHT][WIDTH];

//��Ƥ��줿���2����ʿ�̾�γ����κ�ɸ���Ǽ�����ΰ�
//double projected_ver[VER_NUM][2];
double projected_ver_buf[3][2];


//2��p��q����ľ�����y��ɸ��y�Ǥ���褦������x��ɸ���֤��ؿ�
//eg)
//double p[2] = (1.0, 2.0);
double func1(double *p, double *q, double y){
    double x;
    if(p[1] > q[1]){
        x = ((p[0] * (y - q[1])) + (q[0] * (p[1] - y))) / (p[1] - q[1]);
    }
    if(p[1] < q[1]){
        x = ((q[0] * (y - p[1])) + (p[0] * (q[1] - y))) / (q[1] - p[1]);
    }
    if(p[1] == q[1]){
        //��ʤ�
        printf("\n�����������Ǥ�.\n2��\n(%f, %f)\n(%f, %f)\n��y��ɸ��Ʊ���Ǥ�.\n"
               , p[0], p[1], q[0], q[1]);
        perror(NULL);
        return -1;
    }
    //printf("check x = %f\n", x);
    //printf("check p[0] = %f\n", p[0]);
    return x;
}

//3��a[2] = {x, y},,,��1ľ����ˤ��뤫�ɤ�����Ƚ�ꤹ��ؿ�
//1ľ�����̵�����return 0;
//1ľ����ˤ����return 1;
int lineOrNot(double *a, double *b, double *c){
    if(a[0] == b[0]){
        if(a[0] == c[0]){
            return 1;
        }
        else{
            return 0;
        }
    }
    else{
        if(c[1] == a[1] + ((b[1] - a[1]) / (b[0] - a[0])) * (c[0] - a[0])){
            return 1;
        }
        else{
            return 0;
        }
    }
}


//������3���κ�ɸ��RGB�ˤ���ʤǤʤ��ȥ롼�פǤ��ʤ���
void shading(double *a, double *b, double *c, double *rgb_a, double *rgb_b, double *rgb_c){
    //3����1ľ������¤�Ǥ���Ȥ��ϥ������ǥ��󥰤��Ǥ��ʤ�
    if(lineOrNot(a, b, c) == 1){
        //�ɤ�Ĥ֤�����̵���Τǲ��⤷�ʤ�.
        
        //debug
        /* printf("\n3��\na�κ�ɸ(%f,\t%f)\nb�κ�ɸ(%f,\t%f)\nc�κ�ɸ(%f,\t%f)\n�ϰ�ľ�����3���Ǥ�\n" */
        /*        ,a[0], a[1], b[0], b[1], c[0], c[1]); */
        
    }
    else{
        //y��ɸ���ͤ�����������p������¾������q��r�Ȥ���
        //y��ɸ���礭����r <= p <= q�ν�
        double p[2], q[2], r[2];
        //ˡ���٥��ȥ��̾�����ѹ�����
        double rgb_p[3], rgb_q[3], rgb_r[3];

        if(b[1] <= a[1] && a[1] <= c[1]){
            memcpy(p, a, sizeof(double) * 2);
            memcpy(q, c, sizeof(double) * 2);
            memcpy(r, b, sizeof(double) * 2);
            
            memcpy(rgb_p, rgb_a, sizeof(double) * 3);
            memcpy(rgb_q, rgb_c, sizeof(double) * 3);
            memcpy(rgb_r, rgb_b, sizeof(double) * 3);
        }
        else{
            if(c[1] <= a[1] && a[1] <= b[1]){
                memcpy(p, a, sizeof(double) * 2);
                memcpy(q, b, sizeof(double) * 2);
                memcpy(r, c, sizeof(double) * 2);
                
                memcpy(rgb_p, rgb_a, sizeof(double) * 3);
                memcpy(rgb_q, rgb_b, sizeof(double) * 3);
                memcpy(rgb_r, rgb_c, sizeof(double) * 3);

            }
            else{
                if(a[1] <= b[1] && b[1] <= c[1]){
                    memcpy(p, b, sizeof(double) * 2);
                    memcpy(q, c, sizeof(double) * 2);
                    memcpy(r, a, sizeof(double) * 2);
                    
                    memcpy(rgb_p, rgb_b, sizeof(double) * 3);
                    memcpy(rgb_q, rgb_c, sizeof(double) * 3);
                    memcpy(rgb_r, rgb_a, sizeof(double) * 3);
                }
                else{
                    if(c[1] <= b[1] && b[1] <= a[1]){
                        memcpy(p, b, sizeof(double) * 2);
                        memcpy(q, a, sizeof(double) * 2);
                        memcpy(r, c, sizeof(double) * 2);
                        
                        memcpy(rgb_p, rgb_b, sizeof(double) * 3);
                        memcpy(rgb_q, rgb_a, sizeof(double) * 3);
                        memcpy(rgb_r, rgb_c, sizeof(double) * 3);
                    }
                    else{
                        if(b[1] <= c[1] && c[1] <= a[1]){
                            memcpy(p, c, sizeof(double) * 2);
                            memcpy(q, a, sizeof(double) * 2);
                            memcpy(r, b, sizeof(double) * 2);

                            memcpy(rgb_p, rgb_c, sizeof(double) * 3);
                            memcpy(rgb_q, rgb_a, sizeof(double) * 3);
                            memcpy(rgb_r, rgb_b, sizeof(double) * 3);
                        }
                        else{
                            if(a[1] <= c[1] && c[1] <= b[1]){
                                memcpy(p, c, sizeof(double) * 2);
                                memcpy(q, b, sizeof(double) * 2);
                                memcpy(r, a, sizeof(double) * 2);

                                memcpy(rgb_p, rgb_c, sizeof(double) * 3);
                                memcpy(rgb_q, rgb_b, sizeof(double) * 3);
                                memcpy(rgb_r, rgb_a, sizeof(double) * 3);

                            }
                            else{
                                printf("���顼at2055\n");
                                printf("\na[1]=%f\tb[1]=%f\tc[1]=%f\n", a[1], b[1], c[1]);
                                perror(NULL);
                            }
                        }
                    }
                }
            }
        }

        //debug
        /* printf("\n3���κ�ɸ��\np�κ�ɸ(%f, %f)\nq�κ�ɸ(%f, %f)\nr�κ�ɸ(%f, %f)\n" */
        /*        ,p[0], p[1], q[0], q[1], r[0], r[1]); */
        
        //ʬ���ǽ�ʻ��ѷ�����Ƚ��
        if(p[1] == r[1] || p[1] == q[1]){
            //ʬ��Ǥ��ʤ�

            //debug
            /* printf("\n���ѷ�\np�κ�ɸ(%f, %f)\nq�κ�ɸ(%f, %f)\nr�κ�ɸ(%f, %f)\n��ʬ��Ǥ��ʤ��ΤǤ��Τޤޥ������ǥ���\n" */
            /*        ,p[0], p[1], q[0], q[1], r[0], r[1]); */

            
            //Ĺ����1�θ��������٥��ȥ���������
            //���������٥��ȥ��Ĺ��
            double length_l =
                sqrt(pow(light_dir[0], 2.0) +
                     pow(light_dir[1], 2.0) +
                     pow(light_dir[2], 2.0));
            
            double light_dir_vec[3];
            light_dir_vec[0] = light_dir[0] / length_l;
            light_dir_vec[1] = light_dir[1] / length_l;
            light_dir_vec[2] = light_dir[2] / length_l;
                            
            //2�ѥ�����λ��ѷ�������
            if(p[1] == r[1]){
                //debug
                //printf("\np[1] == r[1]\n");
                //x��ɸ�� p <= r �Ȥʤ�褦��Ĵ��
                if(r[0] <  p[0]){
                    double temp[2];
                    double temp_rgb[3];
                    memcpy(temp, r, sizeof(double) * 2);
                    memcpy(r, p, sizeof(double) * 2);
                    memcpy(p, temp, sizeof(double) * 2);

                    memcpy(temp_rgb, rgb_r, sizeof(double) * 3);
                    memcpy(rgb_r, rgb_p, sizeof(double) * 3);
                    memcpy(rgb_p, temp_rgb, sizeof(double) * 3);
                    
                    //debug
                    /* printf("\n�򴹸��3���κ�ɸ��\np�κ�ɸ(%f, %f)\nq�κ�ɸ(%f, %f)\nr�κ�ɸ(%f, %f)\n" */
                    /*        ,p[0], p[1], q[0], q[1], r[0], r[1]); */
                    
                }
                
                //debug
                if(r[0] == p[0]){
                  perror("���顼at958");
                }
                
                //�������ǥ��󥰽���
                //�������ǥ��󥰤κݤ˲��̤���Ϥ߽Ф�����ʬ��ɤ�������
                //�ʲ��μ�����xy��ɸ���ϰϤ�0 <= x, y <= 256�Ȥ��Ƽ������Ƥ���
                //���ѷ�pqr�򥷥����ǥ���
                //y��ɸ��p <= r
                //debug
                if(r[1] < p[1]){
                    perror("���顼at1855");
                }

                //3��pqr�ˤĤ�����˥������ǥ��󥰤ǿ���̤�
                int i;
                i = ceil(p[1]);
                for(i;
                    p[1] <= i && i <= q[1];
                    i++){

                    //����ʿ�̤���Ϥ߽ФƤ��ʤ����Υ����å�
                    if(0 <= i
                       &&
                       i <= (HEIGHT - 1)){
                           double x1 = func1(p, q, i);
                           double x2 = func1(r, q, i);
                           int j;
                           j = ceil(x1);
                           
                           for(j;
                               x1 <= j && j <= x2 && 0 <= j && j <= (WIDTH - 1);
                               j++){


                               //����ȿ�ͤ�Ŭ��==========================================================================
                               //======================================================================================
                               //���褹������������ζ�����κ�ɸ.
                               double p_or[3];
                               
                               p_or[0] =
                                   (n[0]*(j-(MAX/2)))
                                   *
                                   ((n[0]*A[0]) + (n[1]*A[1]) + (n[2]*A[2]))
                                   /
                                   ((n[0]*(j-(MAX/2))) + (n[1]*(i-(MAX/2))) + n[2]*FOCUS);
                               
                              p_or[1] =
                                   (n[1]*(i-(MAX/2)))
                                   *
                                   ((n[0]*A[0]) + (n[1]*A[1]) + (n[2]*A[2]))
                                   /
                                   ((n[0]*(j-(MAX/2))) + (n[1]*(i-(MAX/2))) + n[2]*FOCUS);
                               
                              p_or[2] =
                                   FOCUS
                                   *
                                   ((n[0]*A[0]) + (n[1]*A[1]) + (n[2]*A[2]))
                                   /
                                   ((n[0]*(j-(MAX/2))) + (n[1]*(i-(MAX/2))) + n[2]*FOCUS);

                               //e�����������p����������֤ظ�����ñ�������٥��ȥ�
                               //���������ϸ����˸���
                               double e[3];
                               e[0] = -1 * p_or[0];
                               e[1] = -1 * p_or[1];
                               e[2] = -1 * p_or[2];

                               //Ĺ����1�ˤ���
                               double length_e = sqrt(pow(e[0], 2.0) + pow(e[1], 2.0) + pow(e[2], 2.0));
                               e[0] = (e[0] / length_e);
                               e[1] = (e[1] / length_e);
                               e[2] = (e[2] / length_e);

                               //i�ϸ����������������p�ؤ����͸���ñ�������٥��ȥ�
                               //ʿ�Ը����Τ������������
                               //const double light_dir[3] = {-1.0, -1.0, 2.0};
                               //���Ѥ���
                               double i_vec[3];
                               i_vec[0] = light_dir[0];
                               i_vec[1] = light_dir[1];
                               i_vec[2] = light_dir[2];

                               //Ĺ����1�ˤ���
                               double length_i = sqrt(pow(i_vec[0], 2.0) + pow(i_vec[1], 2.0) + pow(i_vec[2], 2.0));
                               i_vec[0] = (i_vec[0] / length_i);
                               i_vec[1] = (i_vec[1] / length_i);
                               i_vec[2] = (i_vec[2] / length_i);

                               //debug
                               /* printf("\nlength_i = %f\n", */
                               /*        sqrt(pow(i_vec[0], 2.0) + pow(i_vec[1], 2.0) + pow(i_vec[2], 2.0))); */

                               //s�٥��ȥ��׻�
                               double s[3];
                               s[0] = e[0] - i_vec[0];
                               s[1] = e[1] - i_vec[1];
                               s[2] = e[2] - i_vec[2];

                               //Ĺ����1�ˤ���
                               double s_length =
                                   sqrt(pow(s[0], 2.0) + pow(s[1], 2.0) + pow(s[2], 2.0));
                               s[0] = (s[0] / s_length);
                               s[1] = (s[1] / s_length);
                               s[2] = (s[2] / s_length);

                               //����sn
                               double sn =
                                   ((s[0] * n[0]) + (s[1] * n[1]) + (s[2] * n[2]));
                               
                               if(sn <= 0){
                                   //debug
                                   //printf("\ndebug at 16052\n");
                                   sn = 0;
                                   //exit(0);
                               }

                               //�Ȼ�ȿ�ͤη׻����Ѥ���ˡ���٥��ȥ�ȸ��������٥��ȥ������
                              // ˡ���٥��ȥ�n�ȸ��������٥��ȥ������
                               double ip =
                                   (n[0] * i_vec[0]) + (n[1] * i_vec[1]) + (n[2] * i_vec[2]);
                               
                               if(0 <= ip){
                                   ip = 0;
                               } 
                                     
               

                               //======================================================================================
                               //======================================================================================
                               
 
                               
                               //z��z�Хåե��γ��������ͤ���礭����������Ԥ�ʤ��ʲ��⤷�ʤ���
                               if(z_buf[i][j] < p_or[2]){
                                   //debug
                                   //printf("\n���褵��ʤ����Ǥ� at 1958\n");
                                   //printf("\np_or[2] = %f\n", p_or[2]);
                                   //exit(0);
                               }
                               
                               else{
                                   image[i][j][0] =
                                       (-1 * ip * diffuse_color[0] * light_rgb[0] * MAX)
                                       + (pow(sn, shininess) * specular_color[0] * light_rgb[0] * MAX)
                                       ;
                                   
                                   image[i][j][1] =
                                       (-1 * ip * diffuse_color[1] * light_rgb[1] * MAX)
                                       + (pow(sn, shininess) * specular_color[1] * light_rgb[1] * MAX)
                                       ;
                                   
                                   image[i][j][2] =
                                       (-1 * ip * diffuse_color[2] * light_rgb[2] * MAX)
                                       + (pow(sn, shininess) * specular_color[2] * light_rgb[2] * MAX)
                                       ;
                                   
                                   //z�Хåե��ι���
                                   //debug
                                   //printf("\nz�Хåե��򹹿����ޤ���.\n");
                                   z_buf[i][j] = p_or[2];
                                   //debug
                                   //printf("\nz_buf => %f\n", z_buf[i][j]);
                                   /* if(z_buf[i][j] < 398 || 505 < z_buf[i][j]){ */
                                   /*     printf("\nz�Хåե����ͤ������Ǥ�\n"); */
                                   /*     printf("\nz_buf => %f\n", z_buf[i][j]); */
                                   /*     perror(NULL); */
                                   /*     exit(0); */
                                   /* } */
                               }
                           }
                    }
                    //�Ϥ߽ФƤ���������褷�ʤ�
                    else{}
                }
                
            }
            
            if(p[1] == q[1]){
                //debug
                //printf("\np[1] == q[1]\n");
                //debug
                /* printf("\n���ѷ�\np�κ�ɸ(%f, %f)\nq�κ�ɸ(%f, %f)\nr�κ�ɸ(%f, %f)\n\n" */
                /*        ,p[0], p[1], q[0], q[1], r[0], r[1]); */
                //x��ɸ�� p < q �Ȥʤ�褦��Ĵ��
                if(q[0] <  p[0]){
                    double temp[2];
                    double temp_rgb[3];                   
                    memcpy(temp, q, sizeof(double) * 2);
                    memcpy(q, p, sizeof(double) * 2);
                    memcpy(p, temp, sizeof(double) * 2);

                    memcpy(temp_rgb, rgb_q, sizeof(double) * 3);
                    memcpy(rgb_q, rgb_p, sizeof(double) * 3);
                    memcpy(rgb_p, temp_rgb, sizeof(double) * 3);
                    
                    //debug
                    /* printf("\n�򴹸��3���κ�ɸ��\np�κ�ɸ(%f, %f)\nq�κ�ɸ(%f, %f)\nr�κ�ɸ(%f, %f)\n" */
                    /*        ,p[0], p[1], q[0], q[1], r[0], r[1]); */
                    
                }
                
                //debug
                if(q[0] == p[0]){
                    perror("���顼at1011");
                }
                
                //�������ǥ��󥰽���
                //���ѷ�pqr�򥷥����ǥ���
                //y��ɸ��p <= q
                
                //debug
                if(q[1] < p[1]){
                    perror("���顼at1856");
                }
                
                int i;
                i = ceil(r[1]);
                //debug
                /* printf("\ni = %d\n", i); */
                /* printf("\nr[1] = %f\n", r[1]); */
                /* printf("\np[1] = %f\n", p[1]); */
                //debug
                /* printf("\n���ѷ�\np�κ�ɸ(%f, %f)\nq�κ�ɸ(%f, %f)\nr�κ�ɸ(%f, %f)\n\n" */
                /*        ,p[0], p[1], q[0], q[1], r[0], r[1]); */
            
                for(i;
                    r[1] <= i && i <= p[1];
                    i++){

                    //������ʬ����Ϥ߽ФƤ��ʤ����Υ����å�
                    if( 0 <= i &&
                        i <= (HEIGHT - 1)){
                        double x1 = func1(p, r, i);
                        double x2 = func1(q, r, i);
                        
                        int j;
                        j = ceil(x1);
                        
                        //debug
                        //printf("\nj = %d\nx1 = %f\nx2 = %f\ni = %d\n",j ,x1, x2, i);
                        
                        for(j;
                            x1 <= j && j <= x2 && 0 <= j && j <= (WIDTH - 1);
                            j++){

                            //����ȿ�ͤ�Ŭ��==========================================================================
                            //======================================================================================
                            //���褹������������ζ�����κ�ɸ.
                            double p_or[3];
                            
                            p_or[0] =
                                (n[0]*(j-(MAX/2)))
                                *
                                ((n[0]*A[0]) + (n[1]*A[1]) + (n[2]*A[2]))
                                /
                                ((n[0]*(j-(MAX/2))) + (n[1]*(i-(MAX/2))) + n[2]*FOCUS);
                            
                            p_or[1] =
                                (n[1]*(i-(MAX/2)))
                                *
                                ((n[0]*A[0]) + (n[1]*A[1]) + (n[2]*A[2]))
                                /
                                ((n[0]*(j-(MAX/2))) + (n[1]*(i-(MAX/2))) + n[2]*FOCUS);
                            
                            p_or[2] =
                                FOCUS
                                *
                                ((n[0]*A[0]) + (n[1]*A[1]) + (n[2]*A[2]))
                                /
                                ((n[0]*(j-(MAX/2))) + (n[1]*(i-(MAX/2))) + n[2]*FOCUS);
                            
                            //e�����������p����������֤ظ�����ñ�������٥��ȥ�
                            //���������ϸ����˸���
                            double e[3];
                            e[0] = -1 * p_or[0];
                            e[1] = -1 * p_or[1];
                            e[2] = -1 * p_or[2];
                            
                            //Ĺ����1�ˤ���
                            double length_e = sqrt(pow(e[0], 2.0) + pow(e[1], 2.0) + pow(e[2], 2.0));
                            e[0] = (e[0] / length_e);
                            e[1] = (e[1] / length_e);
                            e[2] = (e[2] / length_e);
                            
                            //i�ϸ����������������p�ؤ����͸���ñ�������٥��ȥ�
                            //ʿ�Ը����Τ������������
                            //const double light_dir[3] = {-1.0, -1.0, 2.0};
                            //���Ѥ���
                            double i_vec[3];
                            i_vec[0] = light_dir[0];
                            i_vec[1] = light_dir[1];
                            i_vec[2] = light_dir[2];
                            
                            //Ĺ����1�ˤ���
                            double length_i = sqrt(pow(i_vec[0], 2.0) + pow(i_vec[1], 2.0) + pow(i_vec[2], 2.0));
                            i_vec[0] = (i_vec[0] / length_i);
                            i_vec[1] = (i_vec[1] / length_i);
                            i_vec[2] = (i_vec[2] / length_i);
                            
                            //s�٥��ȥ��׻�
                            double s[3];
                            s[0] = e[0] - i_vec[0];
                            s[1] = e[1] - i_vec[1];
                            s[2] = e[2] - i_vec[2];
                            
                            //Ĺ����1�ˤ���
                            double s_length =
                                sqrt(pow(s[0], 2.0) + pow(s[1], 2.0) + pow(s[2], 2.0));
                            s[0] = (s[0] / s_length);
                            s[1] = (s[1] / s_length);
                            s[2] = (s[2] / s_length);
                            
                            //����sn
                            double sn = ((s[0] * n[0]) + (s[1] * n[1]) + (s[2] * n[2]));

                            //debug
                            //printf("\nsn = %f\n", sn);
                            //printf("\nn length %f\n", sqrt(pow(n[0], 2.0) + pow(n[1], 2.0) + pow(n[2], 2.0)));
                            //printf("\ns length %f\n", sqrt(pow(s[0], 2.0) + pow(s[1], 2.0) + pow(s[2], 2.0)));
                            
                            if(sn <= 0){
                                //debug
                                //printf("\ndebug at 1606\n");
                                sn = 0;
                                //exit(0);
                            }

                            //�Ȼ�ȿ��
                            // ˡ���٥��ȥ�n�ȸ��������٥��ȥ������
                            double ip =
                                (n[0] * i_vec[0]) +
                                (n[1] * i_vec[1]) +
                                (n[2] * i_vec[2]);
                            
                            if(0 <= ip){
                                ip = 0;
                                //printf("\ndebug at 1550\n");
                                //exit(0);
                            }
                            
                            //======================================================================================
                            //======================================================================================

                            
                            //z��z�Хåե��γ��������ͤ���礭����������Ԥ�ʤ��ʲ��⤷�ʤ���
                            if(z_buf[i][j] < p_or[2]){
                                //debug
                                //printf("\n���褵��ʤ����Ǥ� at 1614\n");
                                //printf("\np_or[2] = %f\n", p_or[2]);
                                //exit(0);
                            }
                        
                            else{
                            
                                image[i][j][0] =
                                    (-1 * ip * diffuse_color[0] * light_rgb[0] * MAX)
                                    + (pow(sn, shininess) * specular_color[0] * light_rgb[0] * MAX)
                                    ;
                                
                                image[i][j][1] =
                                    (-1 * ip * diffuse_color[1] * light_rgb[1] * MAX)
                                    + (pow(sn, shininess) * specular_color[1] * light_rgb[1] * MAX)
                                    ;
                                
                                image[i][j][2] =
                                    (-1 * ip * diffuse_color[2] * light_rgb[2] * MAX)
                                    + (pow(sn, shininess) * specular_color[2] * light_rgb[2] * MAX)
                                    ;

                                /* printf("\n���褷�ޤ���(%f\t%f\t%f)\n" */
                                /*        , image[i][j][0], image[i][j][1], image[i][j][2]); */

                                //z�Хåե��ι���
                                //debug
                                //printf("\nz�Хåե��򹹿����ޤ���.\n");
                                z_buf[i][j] = p_or[2];
                                //debug
                                /* printf("\nz_buf => %f\n", z_buf[i][j]); */
                                /* if(z_buf[i][j] < 400 || 500 < z_buf[i][j]){ */
                                /*     printf("\nz�Хåե����ͤ������Ǥ�\n"); */
                                /*     printf("\nz_buf => %f\n", z_buf[i][j]); */
                                /*     perror(NULL); */
                                /*     exit(0); */
                                /* } */
                            }
                        }
                    }
                    //����ʿ�̤���Ϥ߽Ф���ʬ�����褷�ʤ�
                    else{}      
                }
            }
            
        }
        //ʬ��Ǥ���
        //ʬ�䤷�Ƥ��줾��Ƶ�Ū�˽���
        //ʬ���λ��ѷ���pp2q��pp2r
        else{
            //debug
            /* printf("\n���ѷ�\np�κ�ɸ(%f, %f)\nq�κ�ɸ(%f, %f)\nr�κ�ɸ(%f, %f)\n��ʬ�䤷�ƥ������ǥ���\n" */
            /*        ,p[0], p[1], q[0], q[1], r[0], r[1]); */
            
            double p2[2];

            p2[0] = func1(q, r, p[1]);
            p2[1] = p[1];

            double rgb_p2[3];
            
            //!!!!!!!!!!!!!!!!!!!!!p2��rgb�����ɬ�פ�����!!!!!!!!!!!!!!!!!!!!!!
            //!!!!!!!!!!!!!!!!!!!!!p2��rgb�����ɬ�פ�����!!!!!!!!!!!!!!!!!!!!!!
            //!!!!!!!!!!!!!!!!!!!!!p2��rgb�����ɬ�פ�����!!!!!!!!!!!!!!!!!!!!!!
            //!!!!!!!!!!!!!!!!!!!!!p2��rgb�����ɬ�פ�����!!!!!!!!!!!!!!!!!!!!!!
            
            //p2�Τۤ���p��x��ɸ����礭���ʤ�褦�ˤ���
            if(p2[0] < p[0]){
                double temp[2];
                double temp_rgb[3];
                
                memcpy(temp, p2, sizeof(double) * 2);
                memcpy(p2, p, sizeof(double) * 2);
                memcpy(p, temp, sizeof(double) * 2);

                memcpy(temp_rgb_, rgb_p2, sizeof(double) * 2);
                memcpy(rgb_p2, rgb_p, sizeof(double) * 2);
                memcpy(rgb_p, temp_rgb, sizeof(double) * 2);
            }
            //debug
            /* printf("\np2[2] = (%f\t%f)\n", p2[0], p2[1]); */
            /* printf("\n���ѷ���\n"); */
            /* printf("���ѷ�pp2q = \n(%f\t%f),\n(%f\t%f),\n(%f\t%f)\n", */
            /*        p[0], p[1], p2[0], p2[1], q[0], q[1]); */
            /* printf("���ѷ�pp2r = \n(%f\t%f),\n(%f\t%f),\n(%f\t%f)\n", */
            /*        p[0], p[1], p2[0], p2[1], r[0], r[1]); */
            /* printf("��ʬ�䤷�ƥ������ǥ���\n"); */
            //ʬ�䤷�Ƥ�Ʊ��ʿ�̾�ʤΤ�ˡ���٥��ȥ��
            //ʿ�̾��Ǥ�դ�����Ʊ����Τ�Ȥ���.
            
            shading(p, p2, q, rgb_p, rgb_p2, rgb_q);
            shading(p, p2, r, rgb_p, rgb_p2, rgb_r);
        }
    }
}





/* VRML 2.0 Reader 
 *
 * ver1.1 2005/10/06 Masaaki IIYAMA (bug fix)
 * ver1.0 2005/09/27 Masaaki IIYAMA
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "vrml.h"


/*
/////////////////////////////////////////////////////////////////
*/
#define MWS 256

static int strindex( char *s, char *t)
{
    int         i, j, k;

    for (i = 0; s[i] != '\0'; i++) {
        for (j = i, k = 0;  t[k] != '\0' && s[j] == t[k]; j++, k++) ;
        if (k > 0 && t[k] == '\0')
            return i;
    }
    return -1;
}

static int getword(
                   FILE *fp,
                   char word[],
                   int sl)
{
    int i,c;

    while ( (c = fgetc(fp)) != EOF && ( isspace(c) || c == '#')) {
        if ( c == '#' ) {
            while ( (c = fgetc(fp)) != EOF && c != '\n') ;
            if ( c == EOF ) return (0);
        }
    }
    if ( c == EOF )
        return (0);
    ungetc(c,fp);

    for ( i = 0 ; i < sl - 1 ; i++) {
        word[i] = fgetc(fp);
        if ( isspace(word[i]) )
            break;
    }
    word[i] = '\0';

    return i;
}

static int read_material(
                         FILE *fp,
                         Surface *surface,
                         char *b)
{
    while (getword(fp,b,MWS)>0) {
        if      (strindex(b,"}")>=0) break;
        else if (strindex(b,"diffuseColor") >= 0) {
            getword(fp,b,MWS);
            surface->diff[0] = atof(b);
            getword(fp,b,MWS);
            surface->diff[1] = atof(b);
            getword(fp,b,MWS);
            surface->diff[2] = atof(b);
        }
        else if (strindex(b,"ambientIntensity") >= 0) {
            getword(fp,b,MWS);
            surface->ambi = atof(b);
        }
        else if (strindex(b,"specularColor") >= 0) {
            getword(fp,b,MWS);
            surface->spec[0] = atof(b);
            getword(fp,b,MWS);
            surface->spec[1] = atof(b);
            getword(fp,b,MWS);
            surface->spec[2] = atof(b);
        }
        else if (strindex(b,"shininess") >= 0) {
            getword(fp,b,MWS);
            surface->shine = atof(b);
        }
    }
    return 1;
}

static int count_point(
                       FILE *fp,
                       char *b)
{
    int num=0;
    while (getword(fp,b,MWS)>0) {
        if (strindex(b,"[")>=0) break;
    }
    while (getword(fp,b,MWS)>0) {
        if (strindex(b,"]")>=0) break;
        else {
            num++;
        }
    }
    if ( num %3 != 0 ) {
        fprintf(stderr,"invalid file type[number of points mismatch]\n");
    }
    return num/3;
}

static int read_point(
                      FILE *fp,
                      Polygon *polygon,
                      char *b)
{
    int num=0;
    while (getword(fp,b,MWS)>0) {
        if (strindex(b,"[")>=0) break;
    }
    while (getword(fp,b,MWS)>0) {
        if (strindex(b,"]")>=0) break;
        else {
            polygon->vtx[num++] = atof(b);
        }
    }
    return num/3;
}

static int count_index(
                       FILE *fp,
                       char *b)
{
    int num=0;
    while (getword(fp,b,MWS)>0) {
        if (strindex(b,"[")>=0) break;
    }
    while (getword(fp,b,MWS)>0) {
        if (strindex(b,"]")>=0) break;
        else {
            num++;
        }
    }
    if ( num %4 != 0 ) {
        fprintf(stderr,"invalid file type[number of indices mismatch]\n");
    }
    return num/4;
}

static int read_index(
                      FILE *fp,
                      Polygon *polygon,
                      char *b)
{
    int num=0;
    while (getword(fp,b,MWS)>0) {
        if (strindex(b,"[")>=0) break;
    }
    while (getword(fp,b,MWS)>0) {
        if (strindex(b,"]")>=0) break;
        else {
            polygon->idx[num++] = atoi(b);
            if (num%3 == 0) getword(fp,b,MWS);
        }
    }
    return num/3;
}

int read_one_obj(
                 FILE *fp,
                 Polygon *poly,
                 Surface *surface)
{
    char b[MWS];
    int flag_material = 0;
    int flag_point = 0;
    int flag_index = 0;

    /* initialize surface */
    surface->diff[0] = 1.0;
    surface->diff[1] = 1.0;
    surface->diff[2] = 1.0;
    surface->spec[0] = 0.0;
    surface->spec[1] = 0.0;
    surface->spec[2] = 0.0;
    surface->ambi = 0.0;
    surface->shine = 0.2;

    if ( getword(fp,b,MWS) <= 0) return 0;

    poly->vtx_num = 0;
    poly->idx_num = 0;

    while (flag_material==0 || flag_point==0 || flag_index==0) {
        if      (strindex(b,"Material")>=0) {
            getword(fp,b,MWS);
            flag_material = 1;
        }
        else if (strindex(b,"point")>=0) {
            fprintf(stderr,"Counting... [point]\n");
            poly->vtx_num = count_point(fp, b);
            flag_point = 1;
        }
        else if (strindex(b,"coordIndex")>=0) {
            fprintf(stderr,"Counting... [coordIndex]\n");
            poly->idx_num = count_index(fp, b);
            flag_index = 1;
        }
        else if (getword(fp,b,MWS) <= 0) return 0;
    }

    flag_material = 0;
    flag_point = 0;
    flag_index = 0;

    fseek(fp, 0, SEEK_SET);
    poly->vtx = (double *)malloc(sizeof(double)*3*poly->vtx_num);
    poly->idx = (int *)malloc(sizeof(int)*3*poly->idx_num);
    while (flag_material==0 || flag_point==0 || flag_index==0) {
        if      (strindex(b,"Material")>=0) {
            fprintf(stderr,"Reading... [Material]\n");
            read_material(fp,surface,b);
            flag_material = 1;
        }
        else if (strindex(b,"point")>=0) {
            fprintf(stderr,"Reading... [point]\n");
            read_point(fp,poly,b);
            flag_point = 1;
        }
        else if (strindex(b,"coordIndex")>=0) {
            fprintf(stderr,"Reading... [coordIndex]\n");
            read_index(fp,poly,b);
            flag_index = 1;
        }
        else if (getword(fp,b,MWS) <= 0) return 0;
    }

    return 1;
}		 


int main (int argc, char *argv[])
{
    int i;
    FILE *fp;
    Polygon poly;
    Surface surface;

    fp = fopen(argv[1], "r");
    read_one_obj(fp, &poly, &surface);

    fprintf(stderr,"%d vertice are found.\n",poly.vtx_num);
    fprintf(stderr,"%d triangles are found.\n",poly.idx_num);

    /* i th vertex */
    /* for ( i = 0 ; i < poly.vtx_num ; i++ ) { */
    /*     fprintf(stdout,"%f %f %f # %d th vertex\n",  */
    /*             poly.vtx[i*3+0], poly.vtx[i*3+1], poly.vtx[i*3+2], */
    /*             i); */
    /* } */

    /* i th triangle */
    /* for ( i = 0 ; i < poly.idx_num ; i++ ) { */
    /*     fprintf(stdout,"%d %d %d # %d th triangle\n",  */
    /*             poly.idx[i*3+0], poly.idx[i*3+1], poly.idx[i*3+2], */
    /*             i); */
    /* } */

    /* material info */
    fprintf(stderr, "diffuseColor %f %f %f\n", surface.diff[0], surface.diff[1], surface.diff[2]);
    fprintf(stderr, "specularColor %f %f %f\n", surface.spec[0], surface.spec[1], surface.spec[2]);
    fprintf(stderr, "ambientIntensity %f\n", surface.ambi);
    fprintf(stderr, "shininess %f\n", surface.shine);

    //===================================================================
    //===================================================================
    //===================================================================
    //===================================================================

    FILE *fp_ppm;
    char *fname = FILENAME;

    
    fp_ppm = fopen( fname, "w" );
    //�ե����뤬�����ʤ��ä��Ȥ�
    if( fp_ppm == NULL ){
        printf("%s�ե����뤬�����ޤ���.\n", fname);
        return -1;
    }
    
    //�ե����뤬�������Ȥ�
    else{
        /* fprintf(stderr, "\n�����ĺ����ɸ�ϰʲ�\n"); */
        /* for(int i = 0; i < poly.vtx_num; i++){ */
        /*     //fprintf(stderr, "%f\t%f\t%f\n", ver[i][0], ver[i][1], ver[i][2]); */
        /*     fprintf(stderr, "%f\t%f\t%f\n", poly.vtx[i*3+0], poly.vtx[i*3+1], poly.vtx[i*3+2]); */
        /* } */
        /* fprintf(stderr, "\n"); */
        
        //�����ΰ������
        for(int i = 0; i < 256; i++){
            for(int j = 0; j < 256; j++){
                image[i][j][0] = 0.0 * MAX;
                image[i][j][1] = 0.0 * MAX;
                image[i][j][2] = 0.0 * MAX;
            }
        }

         //z�Хåե�������
        for(int i = 0; i < 256; i++){
            for(int j = 0; j < 256; j++){
                z_buf[i][j] = DBL_MAX;
            }
        }

        //diffuse_color�γ�Ǽ 
        diffuse_color[0] = surface.diff[0];
        diffuse_color[1] = surface.diff[1];
        diffuse_color[2] = surface.diff[2];

        //shininess�γ�Ǽ
        //����������������������������ա�������������������������������
        //�ʼ¸��ڡ������ɲþ���򻲾ȡ�
        //�ƥե������shininess���ͤ�
        //av4 0.5
        //av5 0.5
        //iiyama1997 1.0
        //aa053 1.0
        //av007 0.34
        
        shininess = surface.shine * 128;

        //speculorColor�γ�Ǽ
        specular_color[0] = surface.spec[0];
        specular_color[1] = surface.spec[1];
        specular_color[2] = surface.spec[2];

        //��ĺ����ˡ���٥��ȥ�����
        //���ѷ�i��ˡ���٥��ȥ���������˳�Ǽ����ʥ����Х��ΰ����¸��
        poly_n[poly.idx_num * 3];
        //���ѷ�i��3��A��B��C����ʤ�
        //����3���Ƿ�������뻰�ѷ���ˡ���٥��ȥ�����poly_n�˳�Ǽ���Ƥ���
        for(int i = 0; i < poly.idx_num; i++){
            double A[3], B[3], C[3];
            A[0] = poly.vtx[(poly.idx[i*3+0])*3 + 0];
            A[1] = poly.vtx[(poly.idx[i*3+0])*3 + 1];
            A[2] = poly.vtx[(poly.idx[i*3+0])*3 + 2];
            
            B[0] = poly.vtx[(poly.idx[i*3+1])*3 + 0];
            B[1] = poly.vtx[(poly.idx[i*3+1])*3 + 1];
            B[2] = poly.vtx[(poly.idx[i*3+1])*3 + 2];
            
            C[0] = poly.vtx[(poly.idx[i*3+2])*3 + 0];
            C[1] = poly.vtx[(poly.idx[i*3+2])*3 + 1];
            C[2] = poly.vtx[(poly.idx[i*3+2])*3 + 2];

            //debug
            /* printf("\n3�����������3���κ�ɸ��\n(%f,\t%f,\t%f)\n(%f,\t%f,\t%f)\n(%f,\t%f,\t%f)\n�Ǥ�\n", */
            /*        A[0], A[1], A[2], */
            /*        B[0], B[1], B[2], */
            /*        C[0], C[1], C[2]); */
            
            //�٥��ȥ�AB, AC���鳰�Ѥ�׻�����
            //ˡ���٥��ȥ�n�����
            double AB[3], AC[3], n[3];
            AB[0] = B[0] - A[0];
            AB[1] = B[1] - A[1];
            AB[2] = B[2] - A[2];
            
            AC[0] = C[0] - A[0];
            AC[1] = C[1] - A[1];
            AC[2] = C[2] - A[2];
            
            double n[3];
            n[0] = (AB[1] * AC[2]) - (AB[2] * AC[1]);
            n[1] = (AB[2] * AC[0]) - (AB[0] * AC[2]);
            n[2] = (AB[0] * AC[1]) - (AB[1] * AC[0]);

            //Ĺ����1��Ĵ��
            double length_n =
                sqrt(pow(n[0], 2.0) +
                     pow(n[1], 2.0) +
                     pow(n[2], 2.0));
        
            n[0] = n[0] / length_n;
            n[1] = n[1] / length_n;
            n[2] = n[2] / length_n;
            
            poly_n[i*3 + 0] = n[0];
            poly_n[i*3 + 1] = n[1];
            poly_n[i*3 + 2] = n[2];
        }
        //���ѷ�i��ˡ���٥��ȥ뤬poly_n�˳�Ǽ���줿.

        //�������ǥ���
        //���ѷ����ȤΥ롼��
        for(int i = 0; i < poly.idx_num; i++){
            //������Ʃ����ƽ���
            for(int j = 0; j < 3; j++){ 
                double xp = poly.vtx[(poly.idx[i*3+j])*3 + 0];
                double yp = poly.vtx[(poly.idx[i*3+j])*3 + 1];
                double zp = poly.vtx[(poly.idx[i*3+j])*3 + 2];
                double zi = FOCUS;

                //debug
                //printf("\nxp = %f\typ = %f\tzp = %f\n", xp, yp, zp);

                //debug 
                if(zp == 0){
                    printf("\n(%f\t%f\t%f) i=%d, j=%d\n", xp, yp, zp, i, j);
                    perror("\n���顼0934\n");
                    //break;
                }
                
                double xp2 = xp * (zi / zp);
                double yp2 = yp * (zi / zp);
                double zp2 = zi;
                
                //��ɸ����ʿ�԰�ư
                projected_ver_buf[j][0] = (MAX / 2) + xp2;
                projected_ver_buf[j][1] = (MAX / 2) + yp2;
            }
            
            double a[2], b[2], c[2];
            a[0] = projected_ver_buf[0][0];
            a[1] = projected_ver_buf[0][1];
            b[0] = projected_ver_buf[1][0];
            b[1] = projected_ver_buf[1][1];
            c[0] = projected_ver_buf[2][0];
            c[1] = projected_ver_buf[2][1];
            
            //debug
            /* printf("\n3��\na�κ�ɸ(%f,\t%f)\nb�κ�ɸ(%f,\t%f)\nc�κ�ɸ(%f,\t%f)\n�Υ������ǥ��󥰤�Ԥ��ޤ�.\n" */
            /*        ,a[0], a[1], b[0], b[1], c[0], c[1]); */

            //3��abc����Ƹ�λ��ѷ�i�ˤγ�ĺ����ˡ���٥��ȥ�򤽤줾��׻����ƴؿ�shading�˰����Ϥ�
            //a��b��c��ĺ���ֹ��poly.idx[i*3+j] (j = 1, 2, 3)
            double na[3], nb[3], nc[3];
            double sum_vec[3] = {0, 0, 0};
            int count = 0;
            for(int l = 0; l < 3; l+){
                for(int k = 0; k < poly.idx_num; k++){
                    if(poly.idx[k*3+0] == poly.idx[i*3+0] ||
                       poly.idx[k*3+1] == poly.idx[i*3+0] ||
                       poly.idx[k*3+2] == poly.idx[i*3+0]){
                        sum_vec[0] =  sum_vec[0] + poly_n[k*3+0];
                        sum_vec[1] =  sum_vec[1] + poly_n[k*3+1];
                        sum_vec[2] =  sum_vec[2] + poly_n[k*3+2];
                        count++;
                    }
                    else{}
                }
                switch(l){
                case 0;
                na[0] = sum_vec[0] / count;
                na[1] = sum_vec[1] / count;
                na[2] = sum_vec[2] / count;
                break;

                case 1;
                nb[0] = sum_vec[0] / count;
                nb[1] = sum_vec[1] / count;
                nb[2] = sum_vec[2] / count;
                break;

                case 2;
                nc[0] = sum_vec[0] / count;
                nc[1] = sum_vec[1] / count;
                nc[2] = sum_vec[2] / count;
                break;

                default;
                printf("\n���顼1026\n");
                perror(NULL);
                exit(0);
                }
            }
                              
            //ʿ��i�������λ��ѷ��򥷥����ǥ���
            shading(a, b, c, na, nb, nc);
        }


        
     
        //�إå�������
        fputs(MAGICNUM, fp_ppm);
        fputs("\n", fp_ppm);
        fputs(WIDTH_STRING, fp_ppm);
        fputs(" ", fp_ppm);
        fputs(HEIGHT_STRING, fp_ppm);
        fputs("\n", fp_ppm);
        fputs(MAX_STRING, fp_ppm);
        fputs("\n" ,fp_ppm);

        //image�ν���
        for(int i = 0; i < 256; i++){
            for(int j = 0; j < 256; j++){
                char r[256];
                char g[256];
                char b[256];
                char str[1024];
                sprintf(r, "%d", (int)round(image[i][j][0]));
                sprintf(g, "%d", (int)round(image[i][j][1]));
                sprintf(b, "%d", (int)round(image[i][j][2]));
                sprintf(str, "%s\t%s\t%s\n", r, g, b);
                fputs(str, fp_ppm); 
            }
        }
    }
    fclose(fp_ppm);
    fclose(fp);
    
    printf("\nppm�ե����� %s �κ�������λ���ޤ���.\n", fname );
    return 1;
}

