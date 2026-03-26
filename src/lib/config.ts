/**
 * 全局配置文件
 * 统一管理项目中的常量和配置项
 */

// 分页配置
export const PAGINATION_CONFIG = {
  // 博客文章每页显示数量
  BLOG_ENTRIES_PER_PAGE: 10,
  // 站点动态默认每页显示数量
  NOTES_DEFAULT_PAGE_SIZE: 10,
  // 默认分页大小（与 astro.config.mjs 保持一致）
  DEFAULT_PAGE_SIZE: 8,
} as const;

// 从 astro.config.mjs 获取配置（如果需要的话）
// 注意：在 Astro 中，config 文件的配置通常不能直接在运行时访问
// 这里提供一个统一的配置管理方式


// 导出便捷的获取函数
export const getPageSize = (type: 'blog' | 'notes' = 'blog'): number => {
  switch (type) {
    case 'blog':
      return PAGINATION_CONFIG.BLOG_ENTRIES_PER_PAGE;
    case 'notes':
      return PAGINATION_CONFIG.NOTES_DEFAULT_PAGE_SIZE;
    default:
      return PAGINATION_CONFIG.DEFAULT_PAGE_SIZE;
  }
};

// 类型定义
export type PageType = 'blog' | 'notes';

//网站信息
export const SITE_INFO = {
  // 网站名称
  NAME: 'SiameseBlog',
  SITE_NAME: 'Siamese Blog',
  SUBNAME: 'Siamese（Yuxuan Wu）的个人blog',
  // 网站描述
  DESCRIPTION: 'SiameseBlog是Yuxuan Wu 的私人分享网站，希望能让你觉得有趣',
  // 网站 URL (生产环境)
  URL: 'https://siameseblog.cn',
  AUTHOR: 'Siamese（Yuxuan Wu）',
  // 本地开发 URL
  DEV_URL: 'http://localhost:4321',
  LOGO_IMAGE: '/favicon/logo.png',
  KEY_WORDS: '信息安全与法学,学习思考资源分享,Siamese的生活记录',
  GOOGLE_ANALYTICS_ID: 'G-XXXXXX',  // 需改为你自己的Google Analytics ID
  BAIDU_ANALYTICS_ID: 'XXXXXXXXXX', // 需改为你自己的百度分析ID
  // Twikoo 评论系统 envId
  TWIKOO_ENV_ID: 'twikoojs-navy.vercel.app',
  // 网站初始时间（用于计算运行时长）
  START_DATE: '2025-07-12',
  // ICP 备案信息
  ICP: {
    NUMBER: '备案号xxxxxx',
    URL: 'https://xxxxxxxxxx'
  }
} as const;

// 全局液态玻璃效果
export const UI_CONFIG = {
  ENABLE_GLASS_EFFECT: true,
} as const;

// 获取当前环境的网站URL
export const getSiteUrl = () => {
  // 在构建时使用生产URL，开发时使用开发URL
  return import.meta.env.PUBLIC_ENV === 'production' ? SITE_INFO.URL : SITE_INFO.DEV_URL;
};
