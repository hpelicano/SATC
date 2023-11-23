#ifndef _PSTM_H_
#define _PSTM_H_

#pragma fieldalign shared2 __pstm
typedef struct __pstm
{
   char                            typ[4];
   char                            prod_id[2];
   short                           dpc_num;
   char                            rte_stat[2];
   char                            orig_pro_name[16];
   char                            ast_rtn_pro_name[16];
   char                            router1_name[16];
   char                            router2_name[16];
   char                            msg_orig_ind;
   char                            originator;
   char                            responder;
   struct
   {
      char                            yy[2];
      char                            mm[2];
      char                            dd[2];
   } post_dat;
   char                            term_ln[4];
   char                            term_fiid[4];
   char                            term_id[16];
   char                            term_name_loc[25];
   char                            term_owner_name[22];
   char                            term_city[13];
   char                            term_st[3];
   char                            term_cntry_cde[2];
   char                            term_typ[2];
   char                            brch_id[4];
   char                            orig_crncy_cde[3];
   char                            auth_ind2;
   short                           term_tim_ofst;
   char                            clerk_id[6];
   struct
   {
      char                            grp[4];
      char                            user_id[8];
   } crt_auth;
   char                            seq_num[12];
   char                            pre_auth_seq_num[12];
   char                            invoice_num[10];
   char                            orig_invoice_num[10];
   char                            batch_seq_num[3];
   char                            batch_num[3];
   char                            shift_num[3];
   char                            acq_inst_id_num[11];
   char                            rcv_inst_id_num[11];
   char                            retl_id[19];
   char                            retl_regn[4];
   char                            retl_grp[4];
   char                            retl_sic_cde[4];
   struct
   {
      struct
      {
         char                            tc[2];
         char                            t;
         char                            aa[2];
         char                            c;
      } tran_cde;
      char                            orig[4];
      char                            dest[4];
      char                            crd_ln[4];
      char                            crd_fiid[4];
      struct
      {
         char                            acct_num[19];
      } acct;
      char                            resp_cde[3];
      char                            rsn_cde[2];
      char                            track2[40];
      char                            mbr_num[3];
      char                            exp_dat[4];
      char                            user_fld2;
      long long                       amt_1;
      long long                       amt_2;
      union
      {
         struct
         {
            char                            auth_crncy_cde[3];
            char                            setl_crncy_cde[3];
            char                            auth_conv_rate[8];
            char                            setl_conv_rate[8];
            long long                       conv_dat_tim;
         } mult_crncy;
         char                            user_fld3[30];
      } u_mult_crncy;
      short                           pre_auth_hld;
      char                            apprv_cde_lgth;
      char                            apprv_cde[8];
      char                            ichg_resp[8];
      char                            pseudo_term_id[4];
      char                            dft_capture_flg;
      char                            setl_flg;
      char                            rfrl_phone[20];
      char                            save_acct_typ[2];
      char                            ovrrde_flg;
   } tran;
   struct
   {
      char                            srv[2];
      char                            retl_prog;
      char                            iss[2];
      char                            over_lmt;
      __int32_t                       onl_lmt;
      __int32_t                       offl_lmt;
      char                            pri[16];
      char                            alt1[16];
      char                            alt2[16];
      char                            auth_ind;
      char                            dflt;
      char                            hit_nneg;
      char                            ast_fnd;
   } rte;
   char                            compl_req;
   union
   {
      char                            adj_flg;
      char                            pre_auth_opt;
   } u_adj_flg;
   short                           admin_sec_ind;
   struct
   {
      char                            yy[2];
      char                            mm[2];
      char                            dd[2];
   } tran_dat;
   struct
   {
      char                            hh[2];
      char                            mm[2];
      char                            ss[2];
      char                            tt[2];
   } tran_tim;
   struct
   {
      char                            yy[2];
      char                            mm[2];
      char                            dd[2];
   } acq_ichg_setl_dat;
   struct
   {
      char                            yy[2];
      char                            mm[2];
      char                            dd[2];
   } iss_ichg_setl_dat;
   char                            rvsl_cde[2];
   char                            pt_srv_cond_cde[2];
   char                            pt_srv_entry_mde[3];
   char                            pin[16];
   char                            pin_size[2];
   char                            pin_tries;
   char                            pin_key[16];
   char                            user_key[16];
   char                            pin_frmt;
   char                            pinpad_char;
   short                           ansi_ofst;
   char                            host_trace_num[6];
   long long                       entry_tim;
   long long                       exit_tim;
   long long                       re_entry_tim;
   char                            frwd_inst_id_num[11];
   char                            crd_accpt_id_num[15];
   char                            crd_iss_id_num[11];
   struct
   {
      char                            msg_typ[4];
      struct
      {
         char                            hh[2];
         char                            mm[2];
         char                            ss[2];
         char                            tt[2];
      } trn_tim;
      char                            trn_dat[4];
      char                            tran_seq_num[12];
      char                            b24_post_dat[4];
   } orig_data;
   char                            rea_for_chrgbck[2];
   char                            num_of_chrgbck;
   union
   {
      char                            check_id_typ[2];
      char                            addr_typ[2];
   } u_check_id_typ;
   union
   {
      char                            check_id_num[24];
      struct
      {
         char                            addr[20];
         char                            addr_vrfy_stat;
         char                            csm_de48_device_type[2];
         char                            user_fld1;
      } addr_flds;
   } u_check_id_num;
   char                            state_cde[2];
   char                            birth_dat[6];
   char                            zip_cde[9];
   char                            csm_campana;
   char                            postal_cde[10];
   char                            user_fld10;
   char                            data_flag;
   short                           num_services;
   struct
   {
      char                            typ[2];
      char                            tran_profile;
      char                            elec_followup;
      __int32_t                       flr_lmt;
      __int32_t                       tran_lmt;
   } srvcs[30];
} pstm_def;
#define pstm_def_Size 1174

#endif