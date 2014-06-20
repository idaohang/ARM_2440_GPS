#ifndef __PROT_H__
#define __PROT_H__

extern int packet(unsigned char *p, unsigned char *data, int dataLenth, unsigned char commType, unsigned char comm);
extern int unpacket(unsigned char *p, unsigned char *data, int dataLenth);

#endif	/*__PROT_H__*/
